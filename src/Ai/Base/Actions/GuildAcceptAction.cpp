/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "GuildAcceptAction.h"
#include "Chat.h"
#include "DatabaseEnv.h"
#include "Event.h"
#include "GuildPackets.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotGuildMgr.h"
#include "PlayerbotSecurity.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "StringFormat.h"

bool GuildAcceptAction::Execute(Event event)
{
    WorldPacket p(event.getPacket());
    p.rpos(0);
    Player* inviter = nullptr;
    std::string Invitedname;
    p >> Invitedname;

    if (normalizePlayerName(Invitedname))
        inviter = ObjectAccessor::FindPlayerByName(Invitedname.c_str());

    if (!inviter)
        return false;

    bool accept = true;
    uint32 guildId = inviter->GetGuildId();
    if (!guildId)
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "guild_accept_inviter_not_in_guild", "You are not in a guild!", {}));
        accept = false;
    }
    else if (bot->GetGuildId())
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "guild_accept_already_in_guild", "Sorry, I am in a guild already", {}));
        accept = false;
    }
    else if (!botAI->GetSecurity()->CheckLevelFor(PLAYERBOT_SECURITY_INVITE, false, inviter, true))
    {
        botAI->TellError(PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "guild_accept_declined", "Sorry, I don't want to join your guild :(", {}));
        accept = false;
    }
    else if (sPlayerbotAIConfig.IsArenaTeamBot(bot->GetGUID()))
    {
        accept = false;
    }
    else if (sPlayerbotAIConfig.IsRandomBotAccount(bot->GetSession()->GetAccountId()))
    {
        uint32 maxBots = sPlayerbotAIConfig.maxBotsInRealGuild;
        if (maxBots > 0 && PlayerbotGuildMgr::instance().IsRealGuild(guildId))
        {
            uint32 botCount = PlayerbotGuildMgr::instance().GetGuildBotCount(guildId);
            if (botCount >= maxBots)
            {
                std::string msg = Acore::StringFormat("|cFFFF0000Превышен лимит ботов в гильдии {}/{}|r", botCount, maxBots);
                WorldPacket notif(SMSG_NOTIFICATION, msg.size() + 1);
                notif << msg;
                inviter->GetSession()->SendPacket(&notif);
                accept = false;
            }
        }
    }

    if (accept)
    {
        WorldPackets::Guild::AcceptGuildInvite data = WorldPacket(CMSG_GUILD_ACCEPT);
        bot->GetSession()->HandleGuildAcceptOpcode(data);
    }
    else
    {
        WorldPackets::Guild::GuildDeclineInvitation data = WorldPacket(CMSG_GUILD_DECLINE);
        bot->GetSession()->HandleGuildDeclineOpcode(data);
    }

    return true;
}
