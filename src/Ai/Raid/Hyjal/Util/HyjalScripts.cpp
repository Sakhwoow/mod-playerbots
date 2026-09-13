/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AllCreatureScript.h"
#include "AllMapScript.h"
#include "DynamicObjectScript.h"
#include "EncounterHelpers.h"
#include "HyjalHelpers.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "Playerbots.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "Timer.h"
#include <mutex>

using namespace HyjalSummitHelpers;
using namespace EncounterHelpers;

static Player* GetFirstPlayerSpellTarget(Spell* spell, Unit* caster)
{
    if (!spell || !caster)
        return nullptr;

    if (Unit* unitTarget = spell->m_targets.GetUnitTarget())
        return unitTarget->ToPlayer();

    std::list<TargetInfo> const& targets = *spell->GetUniqueTargetInfo();
    for (TargetInfo const& targetInfo : targets)
    {
        if (Player* target = ObjectAccessor::GetPlayer(*caster, targetInfo.targetGUID))
            return target;
    }

    return nullptr;
}

static bool ShouldInterruptForArchimondeAirBurst(Player* bot, Player* target)
{
    if (!target)
        return false;

    Player* mainTank = GetGroupMainTank(bot);
    if (!mainTank || bot == mainTank)
        return false;

    float distanceToMainTank = bot->GetExactDist2d(mainTank);

    return (target == mainTank || target == bot) &&
           distanceToMainTank < AIR_BURST_SAFE_DISTANCE;
}

// Records the active Rain of Fire dynamic object so that melee bots can avoid it by running
// away from Azgalor or swapping to a Doomguard; the standard FleePosition() logic to avoid aoe
// can take melee in front of Azgalor, resulting in them getting cleaved
class AzgalorRainOfFireScript : public DynamicObjectScript
{
public:
    AzgalorRainOfFireScript() : DynamicObjectScript("AzgalorRainOfFireScript") {}

    void OnUpdate(DynamicObject* dynobj, uint32 /*diff*/) override
    {
        if (dynobj->GetSpellId() != static_cast<uint32>(HyjalSummitSpells::SPELL_RAIN_OF_FIRE))
            return;

        uint32 instanceId = dynobj->GetMap()->GetInstanceId();
        if (GetActiveAzgalorRainOfFire(instanceId))
            return;

        uint32 now = getMSTime();
        auto instanceIt = rainOfFirePosition.find(instanceId);
        if (instanceIt != rainOfFirePosition.end() &&
            getMSTimeDiff(instanceIt->second.spawnTime, now) < RAIN_OF_FIRE_REACQUIRE_DELAY)
        {
            return;
        }

        bool shouldTrackRainOfFire = false;
        Map::PlayerList const& players = dynobj->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive())
                continue;

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("hyjal", BOT_STATE_COMBAT))
                continue;

            shouldTrackRainOfFire = true;
            break;
        }

        if (!shouldTrackRainOfFire)
            return;

        rainOfFirePosition[instanceId] = RainOfFireData{ dynobj->GetPosition(), now };
    }
};

namespace
{
    std::mutex s_doomfireMutex;
    // instanceId → active Doomfire creature GUIDs
    std::unordered_map<uint32, std::vector<ObjectGuid>> s_doomfireGuids;
}

// Maintains a registry of Doomfire creature GUIDs per Hyjal instance.
// Per-tick trail sampling is handled by ArchimondeDoomfireUpdateScript (AllMapScript),
// which runs once per active Hyjal instance instead of once per every creature in the world.
class ArchimondeDoomfireTrailScript : public AllCreatureScript
{
public:
    ArchimondeDoomfireTrailScript() : AllCreatureScript("ArchimondeDoomfireTrailScript") {}

    void OnCreatureAddWorld(Creature* creature) override
    {
        if (creature->GetEntry() != static_cast<uint32>(HyjalSummitNpcs::NPC_DOOMFIRE))
            return;
        if (creature->GetMapId() != HYJAL_SUMMIT_MAP_ID)
            return;

        std::lock_guard<std::mutex> lock(s_doomfireMutex);
        s_doomfireGuids[creature->GetMap()->GetInstanceId()].push_back(creature->GetGUID());
    }

    void OnCreatureRemoveWorld(Creature* creature) override
    {
        if (creature->GetEntry() != static_cast<uint32>(HyjalSummitNpcs::NPC_DOOMFIRE))
            return;

        doomfireLastSampleTime.erase(creature->GetGUID());

        std::lock_guard<std::mutex> lock(s_doomfireMutex);
        uint32 instanceId = creature->GetMap()->GetInstanceId();
        auto it = s_doomfireGuids.find(instanceId);
        if (it == s_doomfireGuids.end())
            return;

        auto& vec = it->second;
        vec.erase(std::remove(vec.begin(), vec.end(), creature->GetGUID()), vec.end());
    }
};

// Fires once per active Hyjal map instance per tick (instead of once per every creature in
// the world). Samples Doomfire NPC positions to build trail data for bot avoidance.
class ArchimondeDoomfireUpdateScript : public AllMapScript
{
public:
    ArchimondeDoomfireUpdateScript() : AllMapScript("ArchimondeDoomfireUpdateScript") {}

    void OnMapUpdate(Map* map, uint32 /*diff*/) override
    {
        if (map->GetId() != HYJAL_SUMMIT_MAP_ID)
            return;

        uint32 const instanceId = map->GetInstanceId();

        std::vector<ObjectGuid> guids;
        {
            std::lock_guard<std::mutex> lock(s_doomfireMutex);
            auto it = s_doomfireGuids.find(instanceId);
            if (it == s_doomfireGuids.end() || it->second.empty())
                return;
            guids = it->second;
        }

        uint32 const now = getMSTime();
        auto& trail = doomfireTrails[instanceId];

        for (ObjectGuid const& guid : guids)
        {
            Creature* creature = map->GetCreature(guid);
            if (!creature)
                continue;

            auto& lastSample = doomfireLastSampleTime[guid];
            if (getMSTimeDiff(lastSample, now) < 500)
                continue;

            lastSample = now;

            DoomfireTrailData data;
            data.position = creature->GetPosition();
            data.recordTime = now;
            trail.push_back(data);

            constexpr float DOOMFIRE_DANGER_RANGE = 10.0f;
            Map::PlayerList const& players = map->GetPlayers();
            for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
            {
                Player* player = it->GetSource();
                if (!player || !player->IsAlive())
                    continue;

                PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
                if (!botAI || !botAI->HasStrategy("hyjal", BOT_STATE_COMBAT) ||
                    creature->GetDistance(player) > DOOMFIRE_DANGER_RANGE)
                {
                    continue;
                }

                botAI->RequestSpellInterrupt();
            }
        }

        // Expire old trail entries once per map tick
        constexpr uint32 TRAIL_DURATION = 18000;
        trail.erase(std::remove_if(trail.begin(), trail.end(),
            [now](DoomfireTrailData const& d)
            {
                return getMSTimeDiff(d.recordTime, now) > TRAIL_DURATION;
            }), trail.end());
    }

    void OnDestroyMap(Map* map) override
    {
        if (map->GetId() != HYJAL_SUMMIT_MAP_ID)
            return;

        std::lock_guard<std::mutex> lock(s_doomfireMutex);
        s_doomfireGuids.erase(map->GetInstanceId());
    }
};

class ArchimondeAirBurstSpellListenerScript : public AllSpellScript
{
public:
    ArchimondeAirBurstSpellListenerScript() :
        AllSpellScript("ArchimondeAirBurstSpellListenerScript") {}

    void OnSpellCast(
        Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
    {
        if (!spell || !caster || !spellInfo)
            return;

        if (spellInfo->Id != static_cast<uint32>(HyjalSummitSpells::SPELL_AIR_BURST))
            return;

        Player* target = GetFirstPlayerSpellTarget(spell, caster);
        if (!target)
            return;

        archimondeAirBurstTargets[caster->GetMap()->GetInstanceId()] =
            AirBurstData{ target->GetGUID(), getMSTime() };

        Map::PlayerList const& players = caster->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive())
                continue;

            PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
            if (!botAI || !botAI->HasStrategy("hyjal", BOT_STATE_COMBAT) ||
                !ShouldInterruptForArchimondeAirBurst(player, target))
            {
                continue;
            }

            botAI->RequestSpellInterrupt();
        }
    }
};

void AddSC_HyjalSummitBotScripts()
{
    new AzgalorRainOfFireScript();
    new ArchimondeDoomfireTrailScript();
    new ArchimondeDoomfireUpdateScript();
    new ArchimondeAirBurstSpellListenerScript();
}
