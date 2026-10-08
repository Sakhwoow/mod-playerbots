/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "ArenaSeasonMgr.h"
#include "ArenaTeam.h"
#include "ArenaTeamFilter.h"
#include "BattleGroundTactics.h"
#include "CharacterCache.h"
#include "Chat.h"
#include "DatabaseEnv.h"
#include "GuildTaskMgr.h"
#include "PerfMonitor.h"
#include "PlayerbotMgr.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"

using namespace Acore::ChatCommands;

// Core's own ArenaTeamFilter has no concept of "bot" at all (core must build without this
// module present) - bot detection only exists here, in mod-playerbots, via
// playerbots_account_type. Wraps an inner core filter (type/"all") and additionally drops any
// team whose captain has a row in that table, i.e. was ever provisioned as a bot of any kind
// (random or guild) - presence of the row is what matters, not which type value it holds.
class RealPlayerArenaTeamFilter : public ArenaTeamFilter
{
public:
    explicit RealPlayerArenaTeamFilter(std::unique_ptr<ArenaTeamFilter> inner) : _inner(std::move(inner)) {}

    ArenaTeamMgr::ArenaTeamContainer Filter(ArenaTeamMgr::ArenaTeamContainer teams) override
    {
        ArenaTeamMgr::ArenaTeamContainer source = _inner ? _inner->Filter(std::move(teams)) : teams;
        ArenaTeamMgr::ArenaTeamContainer result;

        for (auto const& pair : source)
        {
            ArenaTeam* team = pair.second;
            uint32 accountId = sCharacterCache->GetCharacterAccountIdByGuid(team->GetCaptain());
            if (!accountId)
                continue;

            if (QueryResult botRow = PlayerbotsDatabase.Query(
                    "SELECT 1 FROM playerbots_account_type WHERE account_id = {}", accountId))
                continue;

            result[pair.first] = team;
        }

        return result;
    }

private:
    std::unique_ptr<ArenaTeamFilter> _inner;
};

class playerbots_commandscript : public CommandScript
{
public:
    playerbots_commandscript() : CommandScript("playerbots_commandscript") {}

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable playerbotsDebugCommandTable = {
            {"bg", HandleDebugBGCommand, SEC_GAMEMASTER, Console::Yes},
        };

        static ChatCommandTable playerbotsAccountCommandTable = {
            {"setKey", HandleSetSecurityKeyCommand, SEC_PLAYER, Console::No},
            {"link", HandleLinkAccountCommand, SEC_PLAYER, Console::No},
            {"linkedAccounts", HandleViewLinkedAccountsCommand, SEC_PLAYER, Console::No},
            {"unlink", HandleUnlinkAccountCommand, SEC_PLAYER, Console::No},
        };

        static ChatCommandTable playerbotsArenaSeasonCommandTable = {
            {"reward", HandleArenaSeasonRewardRealCommand, SEC_GAMEMASTER, Console::Yes},
        };

        static ChatCommandTable playerbotsCommandTable = {
            {"bot", HandlePlayerbotCommand, SEC_PLAYER, Console::No},
            {"gtask", HandleGuildTaskCommand, SEC_GAMEMASTER, Console::Yes},
            {"pmon", HandlePerfMonCommand, SEC_GAMEMASTER, Console::Yes},
            {"rndbot", HandleRandomPlayerbotCommand, SEC_GAMEMASTER, Console::Yes},
            {"debug", playerbotsDebugCommandTable},
            {"account", playerbotsAccountCommandTable},
            {"arenaseason", playerbotsArenaSeasonCommandTable},
        };

        static ChatCommandTable commandTable = {
            {"playerbots", playerbotsCommandTable},
        };

        return commandTable;
    }

    static bool HandlePlayerbotCommand(ChatHandler* handler, char const* args)
    {
        return PlayerbotMgr::HandlePlayerbotMgrCommand(handler, args);
    }

    static bool HandleRandomPlayerbotCommand(ChatHandler* handler, char const* args)
    {
        return RandomPlayerbotMgr::HandlePlayerbotConsoleCommand(handler, args);
    }

    static bool HandleGuildTaskCommand(ChatHandler* handler, char const* args)
    {
        return GuildTaskMgr::HandleConsoleCommand(handler, args);
    }

    static bool HandlePerfMonCommand(ChatHandler* /*handler*/, char const* args)
    {
        if (!strcmp(args, "reset"))
        {
            sPerfMonitor.Reset();
            return true;
        }

        if (!strcmp(args, "tick"))
        {
            sPerfMonitor.PrintStats(true, false);
            return true;
        }

        if (!strcmp(args, "stack"))
        {
            sPerfMonitor.PrintStats(false, true);
            return true;
        }

        if (!strcmp(args, "toggle"))
        {
            sPlayerbotAIConfig.perfMonEnabled = !sPlayerbotAIConfig.perfMonEnabled;
            if (sPlayerbotAIConfig.perfMonEnabled)
                LOG_INFO("playerbots", "Performance monitor enabled");
            else
                LOG_INFO("playerbots", "Performance monitor disabled");
            return true;
        }

        sPerfMonitor.PrintStats();
        return true;
    }

    static bool HandleDebugBGCommand(ChatHandler* handler, char const* args)
    {
        return BGTactics::HandleConsoleCommand(handler, args);
    }

    // .playerbots arenaseason reward <all|typeList> - exactly like core's own
    // .arena season reward, except arena teams captained by a bot account (random or guild) are
    // excluded first, so titles/mounts/gold only ever land on real players.
    static bool HandleArenaSeasonRewardRealCommand(ChatHandler* handler, char const* args)
    {
        std::string teamsFilterStr = (args && *args) ? args : "all";

        std::unique_ptr<ArenaTeamFilter> baseFilter = ArenaTeamFilterFactoryByUserInput().CreateFilterByUserInput(teamsFilterStr);
        if (!baseFilter)
        {
            handler->PSendSysMessage("Invalid filter. Please check your input.");
            return false;
        }

        if (!sArenaSeasonMgr->CanDeleteArenaTeams())
        {
            handler->PSendSysMessage("Cannot proceed. Make sure there are no active arenas and that rewards exist for the current season.");
            handler->PSendSysMessage("Hint: You can disable the arena queue using the following command: .arena season set state 0");
            return false;
        }

        std::shared_ptr<ArenaTeamFilter> sharedFilter = std::make_shared<RealPlayerArenaTeamFilter>(std::move(baseFilter));

        handler->PSendSysMessage("Distributing rewards for REAL PLAYER arena teams only (types: {})...", teamsFilterStr);
        sArenaSeasonMgr->RewardTeamsForTheSeason(sharedFilter);
        handler->PSendSysMessage("Rewards distributed.");
        return true;
    }

    static bool HandleSetSecurityKeyCommand(ChatHandler* handler, char const* args)
    {
        if (!args || !*args)
        {
            handler->PSendSysMessage("Usage: .playerbots account setKey <securityKey>");
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();
        std::string key = args;

        PlayerbotMgr* mgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);
        if (mgr)
        {
            mgr->HandleSetSecurityKeyCommand(player, key);
            return true;
        }
        else
        {
            handler->PSendSysMessage("PlayerbotMgr instance not found.");
            return false;
        }
    }

    static bool HandleLinkAccountCommand(ChatHandler* handler, char const* args)
    {
        if (!args || !*args)
            return false;

        char* accountName = strtok((char*)args, " ");
        char* key = strtok(nullptr, " ");

        if (!accountName || !key)
        {
            handler->PSendSysMessage("Usage: .playerbots account link <accountName> <securityKey>");
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();

        PlayerbotMgr* mgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);
        if (mgr)
        {
            mgr->HandleLinkAccountCommand(player, accountName, key);
            return true;
        }
        else
        {
            handler->PSendSysMessage("PlayerbotMgr instance not found.");
            return false;
        }
    }

    static bool HandleViewLinkedAccountsCommand(ChatHandler* handler, char const* /*args*/)
    {
        Player* player = handler->GetSession()->GetPlayer();

        PlayerbotMgr* mgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);
        if (mgr)
        {
            mgr->HandleViewLinkedAccountsCommand(player);
            return true;
        }
        else
        {
            handler->PSendSysMessage("PlayerbotMgr instance not found.");
            return false;
        }
    }

    static bool HandleUnlinkAccountCommand(ChatHandler* handler, char const* args)
    {
        if (!args || !*args)
            return false;

        char* accountName = strtok((char*)args, " ");
        if (!accountName)
        {
            handler->PSendSysMessage("Usage: .playerbots account unlink <accountName>");
            return false;
        }

        Player* player = handler->GetSession()->GetPlayer();

        PlayerbotMgr* mgr = PlayerbotsMgr::instance().GetPlayerbotMgr(player);
        if (mgr)
        {
            mgr->HandleUnlinkAccountCommand(player, accountName);
            return true;
        }
        else
        {
            handler->PSendSysMessage("PlayerbotMgr instance not found.");
            return false;
        }
    }
};

void AddPlayerbotsCommandscripts() { new playerbots_commandscript(); }
