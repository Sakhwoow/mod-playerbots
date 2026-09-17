/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_PLAYERBOTGUILDMGR_H
#define PLAYERBOTS_PLAYERBOTGUILDMGR_H

#include "Guild.h"
#include "Player.h"
#include "PlayerbotAI.h"
// #include <chrono>  // was needed for realPlayerCheckedAt — moved to mod-guild-bots

class PlayerbotGuildMgr
{
public:
    static PlayerbotGuildMgr& instance()
    {
        static PlayerbotGuildMgr instance;

        return instance;
    }

    void Init();
    std::string AssignToGuild(Player* player);
    void LoadGuildNames();
    void ValidateGuildCache();
    void ResetGuildCache();
    bool CreateGuild(Player* player, std::string guildName);
    void OnGuildUpdate  (Guild* guild);
    bool SetGuildEmblem(uint32 guildId);
    void DeleteRandomBotGuilds();
    bool IsRealGuild(uint32 guildId);
    bool IsRealGuild(Player* bot);

    // void IncrementGuildBotCount(uint32 guildId);  // moved to mod-guild-bots
    // void DecrementGuildBotCount(uint32 guildId);  // moved to mod-guild-bots
    // uint32 GetGuildBotCount(uint32 guildId);      // moved to mod-guild-bots
    // void SetHasRealPlayer(uint32 guildId, bool value);  // moved to mod-guild-bots

private:
    // void LoadGuildBotCounts();                        // moved to mod-guild-bots
    // bool _guildBotCountLoaded = false;               // moved to mod-guild-bots
    // std::unordered_map<uint32, uint32> _guildBotCount;  // moved to mod-guild-bots
    PlayerbotGuildMgr() = default;
    ~PlayerbotGuildMgr() = default;

    PlayerbotGuildMgr(PlayerbotGuildMgr const&) = delete;
    PlayerbotGuildMgr& operator=(PlayerbotGuildMgr const&) = delete;

    PlayerbotGuildMgr(PlayerbotGuildMgr&&) = delete;
    PlayerbotGuildMgr& operator=(PlayerbotGuildMgr&&) = delete;

    std::unordered_map<std::string, bool> _guildNames;

    struct GuildCache
    {
        std::string name;
        uint8 status;
        uint32 maxMembers = 0;
        uint32 memberCount = 0;
        uint8 faction = 0;
        bool hasRealPlayer = false;
        // std::chrono::steady_clock::time_point realPlayerCheckedAt = {};  // removed: TTL cache moved to mod-guild-bots
    };
    std::unordered_map<uint32 , GuildCache> _guildCache;
    std::vector<std::string> _shuffled_guild_keys;
};

void PlayerBotsGuildValidationScript();

#endif
