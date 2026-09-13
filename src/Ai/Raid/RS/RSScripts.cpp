/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "RSScripts.h"
#include "AllCreatureScript.h"
#include "AllMapScript.h"
#include "Map.h"
#include "Player.h"
#include "Playerbots.h"
#include "RSActions.h"
#include "RSTriggers.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "Timer.h"
#include "UnitScript.h"
#include "Vehicle.h"
#include <algorithm>
#include <cmath>
#include <mutex>

namespace RubySanctumHelpers
{
    static std::mutex stateMutex;
    static std::unordered_map<uint32, RsInstanceState> instanceStates;

    RsInstanceState& RsState(uint32 instanceId)
    {
        std::lock_guard<std::mutex> lock(stateMutex);
        return instanceStates[instanceId];
    }

    void ResetInstance(uint32 instanceId)
    {
        std::lock_guard<std::mutex> lock(stateMutex);
        instanceStates.erase(instanceId);
    }
}

static uint8 RsHalionReadCorporealityIndex(Creature* creature)
{
    using namespace RubySanctumHelpers;
    for (uint8 i = 0; i <= 10; ++i)
    {
        if (creature->HasAura(HALION_CORPOREALITY_AURAS[i]))
            return i;
    }
    return 5;
}

// Registry of relevant Ruby Sanctum creature GUIDs per instance.
// Populated by RsHalionCreatureTrackerScript (AllCreatureScript add/remove hooks)
// and consumed by RsHalionRootScript (AllMapScript per-tick update).
namespace
{
    std::mutex s_rsCreatureMutex;
    std::unordered_map<uint32, std::vector<ObjectGuid>> s_rsTrackedCreatures; // instanceId → GUIDs
}

static void RsMeteorMarkProcess(Creature* mark)
{
    if (!mark->IsAlive())
        return;

    RubySanctumHelpers::RsInstanceState& instState = RubySanctumHelpers::RsState(mark->GetMap()->GetInstanceId());

    if (!instState.seenMeteorMarks.insert(mark->GetGUID()).second)
        return;

    RubySanctumHelpers::MeteorPingPong& state = instState.meteorPingPong;
    state.lastCastTime = getMSTime();

    bool const tankMeteor = mark->GetExactDist2d(RS_HALION_TANK_POSITION.GetPositionX(), RS_HALION_TANK_POSITION.GetPositionY()) <= 10.0f;
    bool const tankEscapeMeteor = mark->GetExactDist2d(RS_HALION_TANK_METEOR_SPOT.GetPositionX(), RS_HALION_TANK_METEOR_SPOT.GetPositionY()) <= 10.0f;
    if (tankMeteor)
        state.tankMeteorTime = getMSTime();
    else if (tankEscapeMeteor)
        state.tankReturnTime = getMSTime();
    else
        ++state.count;
}

// Fires once per active Ruby Sanctum map instance per tick instead of once per every creature
// in the world. Handles Halion root, cutter timing, and living add tracking.
class RsHalionRootScript : public AllMapScript
{
public:
    RsHalionRootScript() : AllMapScript("RsHalionRootScript") {}

    void OnMapUpdate(Map* map, uint32 /*diff*/) override
    {
        if (map->GetId() != RS_MAP_RUBY_SANCTUM)
            return;

        uint32 const instanceId = map->GetInstanceId();

        std::vector<ObjectGuid> guids;
        {
            std::lock_guard<std::mutex> lock(s_rsCreatureMutex);
            auto it = s_rsTrackedCreatures.find(instanceId);
            if (it == s_rsTrackedCreatures.end())
                return;
            guids = it->second;
        }

        for (ObjectGuid const& guid : guids)
        {
            Creature* creature = map->GetCreature(guid);
            if (!creature)
                continue;

            switch (creature->GetEntry())
            {
                case NPC_HALION:
                    HandleHalionRoot(creature);
                    break;
                case NPC_TWILIGHT_HALION:
                    HandleTwilightHalionRoot(creature);
                    break;
                case NPC_ORB_CARRIER:
                    HandleCutterTiming(creature);
                    break;
                case NPC_LIVING_INFERNO:
                case NPC_LIVING_EMBER:
                    if (creature->IsAlive())
                        RubySanctumHelpers::RsState(instanceId).portalAddGate.lastAddAliveTime = getMSTime();
                    break;
                default:
                    break;
            }
        }
    }

    void OnDestroyMap(Map* map) override
    {
        if (map->GetId() != RS_MAP_RUBY_SANCTUM)
            return;

        std::lock_guard<std::mutex> lock(s_rsCreatureMutex);
        s_rsTrackedCreatures.erase(map->GetInstanceId());
    }

private:
    void HandleCutterTiming(Creature* carrier)
    {
        uint32 const instanceId = carrier->GetMap()->GetInstanceId();
        RubySanctumHelpers::CutterTiming& state = RubySanctumHelpers::RsState(instanceId).cutterTiming;

        bool firing = false;
        Unit* tracked = nullptr;
        if (Vehicle* vehicle = carrier->GetVehicleKit())
        {
            tracked = vehicle->GetPassenger(0);
            for (int8 seat = 0; seat < 4; ++seat)
            {
                if (Unit* orb = vehicle->GetPassenger(seat); orb && orb->HasAura(SPELL_TWILIGHT_PULSE_PERIODIC))
                {
                    firing = true;
                    break;
                }
            }
        }

        if (firing && !state.active)
            state.lastShootTime = getMSTime();

        state.active = firing;

        if (tracked)
        {
            float const angle = std::atan2(tracked->GetPositionY() - carrier->GetPositionY(),
                                           tracked->GetPositionX() - carrier->GetPositionX());
            if (state.hasOrbAngle)
            {
                float const delta = RsAngleDiff(angle, state.lastOrbAngle);
                if (std::fabs(delta) > 0.0005f)
                    state.spinSign = delta > 0.0f ? 1.0f : -1.0f;
            }
            state.lastOrbAngle = angle;
            state.hasOrbAngle = true;
        }
        else
        {
            state.hasOrbAngle = false;
        }
    }

    void HandleTankAuras(Creature* creature)
    {
        uint32 const instanceId = creature->GetMap()->GetInstanceId();
        uint32 const now = getMSTime();

        uint32& last = RubySanctumHelpers::RsState(instanceId).tankAuraLastApply;
        if (last != 0 && getMSTimeDiff(last, now) < 2000)
            return;
        last = now;

        constexpr uint32 RS_BOT_DMG_BONUS_SPELL = 71188;

        Map::PlayerList const& players = creature->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() || !GET_PLAYERBOT_AI(player))
                continue;

            if (int const bonus = sPlayerbotAIConfig.RSBotDamageBonus; bonus > 0)
            {
                if (!player->HasAura(RS_BOT_DMG_BONUS_SPELL))
                    if (Aura* aura = player->AddAura(RS_BOT_DMG_BONUS_SPELL, player))
                        if (AuraEffect* eff = aura->GetEffect(0))
                            eff->ChangeAmount(bonus);
            }

            if (!PlayerbotAI::IsTank(player))
                continue;

            if (int const reduction = sPlayerbotAIConfig.RSBotDamageTaken; reduction > 0)
            {
                if (!player->HasAura(RS_SPELL_PAIN_SUPPRESION))
                    if (Aura* aura = player->AddAura(RS_SPELL_PAIN_SUPPRESION, player))
                        if (AuraEffect* eff = aura->GetEffect(0))
                            eff->ChangeAmount(-reduction);
            }
        }
    }

    void HandleAddBuff(Creature* creature)
    {
        uint32 const instanceId = creature->GetMap()->GetInstanceId();
        uint32 const now = getMSTime();

        uint32& last = RubySanctumHelpers::RsState(instanceId).addBuffLastApply;
        if (last != 0 && getMSTimeDiff(last, now) < 2000)
            return;
        last = now;

        bool addsAlive = false;
        Map::PlayerList const& players = creature->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player)
                continue;
            if (Creature* add = player->FindNearestCreature(NPC_LIVING_INFERNO, 200.0f); add && add->IsAlive())
            {
                addsAlive = true;
                break;
            }
            if (Creature* add = player->FindNearestCreature(NPC_LIVING_EMBER, 200.0f); add && add->IsAlive())
            {
                addsAlive = true;
                break;
            }
        }

        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() || !GET_PLAYERBOT_AI(player))
                continue;
            bool const wantEmpowered = addsAlive && !player->HasAura(SPELL_TWILIGHT_REALM);
            if (wantEmpowered && !player->HasAura(RS_SPELL_EMPOWERED_BLOOD))
                player->AddAura(RS_SPELL_EMPOWERED_BLOOD, player);
            else if (!wantEmpowered && player->HasAura(RS_SPELL_EMPOWERED_BLOOD))
                player->RemoveAura(RS_SPELL_EMPOWERED_BLOOD);
        }
    }

    void HandleP3TankRescue(Creature* creature)
    {
        uint32 const now = getMSTime();
        uint32 const rescueMs = 5000;
        float const bossHp = creature->GetHealthPct();

        auto& grantStart = RubySanctumHelpers::RsState(creature->GetMap()->GetInstanceId()).p3RescueGrant;

        Map::PlayerList const& players = creature->GetMap()->GetPlayers();

        bool physicalHealerAlive = false;
        bool twilightHealerAlive = false;
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() || !PlayerbotAI::IsHeal(player))
                continue;
            if (RsHalionInTwilight(player))
                twilightHealerAlive = true;
            else
                physicalHealerAlive = true;
        }

        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() || !GET_PLAYERBOT_AI(player) || !PlayerbotAI::IsTank(player))
                continue;

            bool const tankTwilight = RsHalionInTwilight(player);
            bool const healerInRealm = tankTwilight ? twilightHealerAlive : physicalHealerAlive;

            ObjectGuid const guid = player->GetGUID();

            if (!tankTwilight && bossHp >= 46.0f)
            {
                player->AddAura(RS_SPELL_MAGIC_BARRIER, player);
                grantStart.erase(guid);
                continue;
            }

            if (bossHp > 50.0f)
                continue;

            auto granted = grantStart.find(guid);

            if (granted != grantStart.end())
            {
                if (getMSTimeDiff(granted->second, now) >= rescueMs)
                {
                    player->RemoveAura(RS_SPELL_MAGIC_BARRIER);
                    grantStart.erase(granted);
                }
                else
                    player->AddAura(RS_SPELL_MAGIC_BARRIER, player);
                continue;
            }

            if (!player->HealthAbovePct(50) && healerInRealm)
            {
                player->AddAura(RS_SPELL_MAGIC_BARRIER, player);
                grantStart[guid] = now;
            }
        }
    }

    void HandleBreathGodMode(Creature* boss, uint32 const* breathIds, size_t breathCount, bool twilight)
    {
        uint32 const now = getMSTime();
        uint32 const holdMs = 1000;

        bool casting = false;
        for (size_t i = 0; i < breathCount; ++i)
        {
            if (boss->FindCurrentSpellBySpellId(breathIds[i]))
            {
                casting = true;
                break;
            }
        }

        RubySanctumHelpers::RsInstanceState& state = RubySanctumHelpers::RsState(boss->GetMap()->GetInstanceId());
        std::unordered_map<ObjectGuid, uint32>& grantStart =
            twilight ? state.breathTwilightGrant : state.breathPhysicalGrant;

        Map::PlayerList const& players = boss->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() || !GET_PLAYERBOT_AI(player) || PlayerbotAI::IsTank(player) || RsHalionInTwilight(player) != twilight)
                continue;

            ObjectGuid const guid = player->GetGUID();
            if (casting)
            {
                player->AddAura(RS_SPELL_MAGIC_BARRIER, player);
                grantStart[guid] = now;
                continue;
            }

            auto granted = grantStart.find(guid);
            if (granted == grantStart.end())
                continue;

            if (getMSTimeDiff(granted->second, now) >= holdMs)
            {
                player->RemoveAura(RS_SPELL_MAGIC_BARRIER);
                grantStart.erase(granted);
            }
            else
                player->AddAura(RS_SPELL_MAGIC_BARRIER, player);
        }
    }

    void HandleCombustionDispel(Creature* physical)
    {
        Map::PlayerList const& players = physical->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() || GET_PLAYERBOT_AI(player))
                continue;
            if (!RsHalionHasCombustion(player) || !RsHalionCombustionPlayerSafe(player))
                continue;

            player->RemoveAurasDueToSpell(SPELL_FIERY_COMBUSTION, ObjectGuid::Empty, 0, AURA_REMOVE_BY_EXPIRE);
            player->RemoveAurasDueToSpell(SPELL_MARK_OF_COMBUSTION, ObjectGuid::Empty, 0, AURA_REMOVE_BY_EXPIRE);
        }
    }

    void HandleConsumptionGodMode(Creature* twilight)
    {
        if (!twilight->GetMap()->IsHeroic())
            return;

        uint32 const now = getMSTime();
        uint32 const holdMs = 2000;

        auto& grantStart = RubySanctumHelpers::RsState(twilight->GetMap()->GetInstanceId()).consumptionGrant;

        Map::PlayerList const& players = twilight->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() || !GET_PLAYERBOT_AI(player))
                continue;

            ObjectGuid const guid = player->GetGUID();
            if (player->HasAura(SPELL_SOUL_CONSUMPTION))
            {
                player->AddAura(RS_SPELL_MAGIC_BARRIER, player);
                grantStart[guid] = now;
                continue;
            }

            auto granted = grantStart.find(guid);
            if (granted == grantStart.end())
                continue;

            if (getMSTimeDiff(granted->second, now) >= holdMs)
            {
                player->RemoveAura(RS_SPELL_MAGIC_BARRIER);
                grantStart.erase(granted);
            }
            else
                player->AddAura(RS_SPELL_MAGIC_BARRIER, player);
        }
    }

    void HandleConsumptionDispel(Creature* twilight)
    {
        float const bossX = twilight->GetPositionX();
        float const bossY = twilight->GetPositionY();

        Map::PlayerList const& players = twilight->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (!player || !player->IsAlive() || GET_PLAYERBOT_AI(player))
                continue;
            if (!RsHalionHasConsumption(player))
                continue;

            float const distToBoss = player->GetExactDist2d(bossX, bossY);
            if (distToBoss < RS_HALION_CONSUMPTION_OUT_DIST - RS_HALION_CONSUMPTION_OUT_REACH)
                continue;

            player->RemoveAurasDueToSpell(SPELL_SOUL_CONSUMPTION, ObjectGuid::Empty, 0, AURA_REMOVE_BY_EXPIRE);
            player->RemoveAurasDueToSpell(SPELL_MARK_OF_CONSUMPTION, ObjectGuid::Empty, 0, AURA_REMOVE_BY_EXPIRE);
        }
    }

    void HandleTwilightHalionRoot(Creature* creature)
    {
        RubySanctumHelpers::RsInstanceState& instState =
            RubySanctumHelpers::RsState(creature->GetMap()->GetInstanceId());

        if (creature->IsAlive() && creature->IsInCombat())
        {
            RubySanctumHelpers::HalionCorporeality& corp = instState.halionCorporeality;
            corp.twilightIndex = RsHalionReadCorporealityIndex(creature);
            corp.twilightStamp = getMSTime();
            instState.bossHealth = { uint8(creature->GetHealthPct()), getMSTime() };
        }

        RubySanctumHelpers::CutterTiming& state = instState.cutterTiming;
        if (state.bossGuid != creature->GetGUID())
        {
            state = RubySanctumHelpers::CutterTiming{};
            state.bossGuid = creature->GetGUID();
        }
        if (state.encounterStart == 0 && creature->IsInCombat())
            state.encounterStart = getMSTime();

        bool twilightTankAlive = false;
        Map::PlayerList const& players = creature->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (player && player->IsAlive() && PlayerbotAI::IsTank(player) && RsHalionInTwilight(player))
            {
                twilightTankAlive = true;
                break;
            }
        }

        bool const shouldRoot = creature->IsAlive() && creature->IsInCombat() && twilightTankAlive;
        bool const rooted = creature->HasUnitState(UNIT_STATE_ROOT);

        if (shouldRoot && !rooted)
            creature->SetControlled(true, UNIT_STATE_ROOT);
        else if (!shouldRoot && rooted)
            creature->SetControlled(false, UNIT_STATE_ROOT);

        if (shouldRoot)
        {
            Position const& home = creature->GetHomePosition();
            if (creature->GetExactDist2d(home.GetPositionX(), home.GetPositionY()) > 3.0f)
                creature->NearTeleportTo(home.GetPositionX(), home.GetPositionY(),
                                         home.GetPositionZ(), home.GetOrientation());
        }

        if (creature->IsAlive() && creature->IsInCombat())
        {
            HandleTankAuras(creature);
            HandleAddBuff(creature);
            static uint32 const darkBreathIds[] = {
                SPELL_DARK_BREATH, SPELL_DARK_BREATH_25N, SPELL_DARK_BREATH_10H, SPELL_DARK_BREATH_25H
            };
            HandleBreathGodMode(creature, darkBreathIds, sizeof(darkBreathIds) / sizeof(darkBreathIds[0]), true);
            HandleConsumptionDispel(creature);
            HandleConsumptionGodMode(creature);
        }
    }

    void HandleHalionRoot(Creature* creature)
    {
        uint32 const instanceId = creature->GetMap()->GetInstanceId();

        bool const fightActive = creature->IsAlive() && creature->IsInCombat();

        if (!fightActive)
        {
            if (creature->HasUnitState(UNIT_STATE_ROOT))
                creature->SetControlled(false, UNIT_STATE_ROOT);
            RubySanctumHelpers::ResetInstance(instanceId);
            return;
        }

        RubySanctumHelpers::RsInstanceState& instState = RubySanctumHelpers::RsState(instanceId);

        RubySanctumHelpers::HalionCorporeality& corp = instState.halionCorporeality;
        corp.physicalIndex = RsHalionReadCorporealityIndex(creature);
        corp.physicalStamp = getMSTime();
        corp.physicalGuid = creature->GetGUID();
        instState.bossHealth = { uint8(creature->GetHealthPct()), getMSTime() };

        HandleTankAuras(creature);
        HandleAddBuff(creature);

        static uint32 const flameBreathIds[] = {
            SPELL_FLAME_BREATH, SPELL_FLAME_BREATH_ALT1, SPELL_FLAME_BREATH_ALT2, SPELL_FLAME_BREATH_ALT3,
            SPELL_FLAME_BREATH_ALT4, SPELL_FLAME_BREATH_ALT5, SPELL_FLAME_BREATH_ALT6
        };
        HandleBreathGodMode(creature, flameBreathIds, sizeof(flameBreathIds) / sizeof(flameBreathIds[0]), false);
        HandleCombustionDispel(creature);

        bool const physicalActive = creature->GetDisplayId() == creature->GetNativeDisplayId();

        if (!physicalActive)
        {
            if (creature->HasUnitState(UNIT_STATE_ROOT))
                creature->SetControlled(false, UNIT_STATE_ROOT);
            return;
        }

        if (creature->GetMap()->IsHeroic() && !creature->HealthAbovePct(51))
            HandleP3TankRescue(creature);

        uint32 const now = getMSTime();
        uint32& last = instState.halionRootLastScan;
        if (last != 0 && getMSTimeDiff(last, now) < 1000)
            return;
        last = now;

        bool mainTankAlive = false;
        Map::PlayerList const& players = creature->GetMap()->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* player = it->GetSource();
            if (player && player->IsAlive() && PlayerbotAI::IsMainTank(player))
            {
                mainTankAlive = true;
                break;
            }
        }

        bool const shouldRoot = mainTankAlive;
        bool const rooted = creature->HasUnitState(UNIT_STATE_ROOT);

        if (shouldRoot && !rooted)
            creature->SetControlled(true, UNIT_STATE_ROOT);
        else if (!shouldRoot && rooted)
            creature->SetControlled(false, UNIT_STATE_ROOT);
    }
};

// Tracks per-instance Ruby Sanctum creature GUIDs so RsHalionRootScript (AllMapScript) can
// iterate only the relevant creatures instead of every creature in the world.
// Also handles one-time meteor mark processing on creature spawn.
class RsHalionCreatureTrackerScript : public AllCreatureScript
{
public:
    RsHalionCreatureTrackerScript() : AllCreatureScript("RsHalionCreatureTrackerScript") {}

    void OnCreatureAddWorld(Creature* creature) override
    {
        if (creature->GetMapId() != RS_MAP_RUBY_SANCTUM)
            return;

        uint32 const entry = creature->GetEntry();

        if (entry == NPC_METEOR_STRIKE_MARK)
        {
            RsMeteorMarkProcess(creature);
            return;
        }

        if (entry != NPC_HALION && entry != NPC_TWILIGHT_HALION &&
            entry != NPC_ORB_CARRIER && entry != NPC_LIVING_INFERNO &&
            entry != NPC_LIVING_EMBER)
            return;

        std::lock_guard<std::mutex> lock(s_rsCreatureMutex);
        s_rsTrackedCreatures[creature->GetMap()->GetInstanceId()].push_back(creature->GetGUID());
    }

    void OnCreatureRemoveWorld(Creature* creature) override
    {
        if (creature->GetMapId() != RS_MAP_RUBY_SANCTUM)
            return;

        uint32 const entry = creature->GetEntry();
        if (entry != NPC_HALION && entry != NPC_TWILIGHT_HALION &&
            entry != NPC_ORB_CARRIER && entry != NPC_LIVING_INFERNO &&
            entry != NPC_LIVING_EMBER)
            return;

        std::lock_guard<std::mutex> lock(s_rsCreatureMutex);
        uint32 const instanceId = creature->GetMap()->GetInstanceId();
        auto it = s_rsTrackedCreatures.find(instanceId);
        if (it == s_rsTrackedCreatures.end())
            return;

        auto& vec = it->second;
        vec.erase(std::remove(vec.begin(), vec.end(), creature->GetGUID()), vec.end());
    }
};

// Dark Breath (Twilight Halion) immunity for real players in the twilight realm.
//
// HandleBreathGodMode gives RS_SPELL_MAGIC_BARRIER to bot non-tanks while the breath
// is casting — but skips real players entirely. If a bot tank is alive in the twilight
// realm, the real player is never the intended target; this hook zeroes the damage as a
// safety net for the gap between fixate updates and the breath actually landing.
class RsHalionDarkBreathImmunity : public UnitScript
{
public:
    RsHalionDarkBreathImmunity() : UnitScript("RsHalionDarkBreathImmunity") { }

    void ModifySpellDamageTaken(Unit* target, Unit* /*attacker*/, int32& damage,
                                SpellInfo const* spellInfo) override
    {
        if (!target || !spellInfo || damage <= 0)
            return;
        if (!target->IsPlayer())
            return;

        uint32 const id = spellInfo->Id;
        if (id != SPELL_DARK_BREATH && id != SPELL_DARK_BREATH_25N &&
            id != SPELL_DARK_BREATH_10H && id != SPELL_DARK_BREATH_25H)
            return;

        Player* player = target->ToPlayer();
        // Bot non-tanks already get RS_SPELL_MAGIC_BARRIER from HandleBreathGodMode.
        if (sPlayerbotsMgr.GetPlayerbotAI(player))
            return;

        // Only protect real players who are in the twilight realm.
        if (!RsHalionInTwilight(player))
            return;

        // Zero damage only when a bot tank is present in the twilight realm and can absorb
        // the breath instead. If no bot tank exists the real player may be tanking intentionally.
        Map* map = player->GetMap();
        if (!map)
            return;

        Map::PlayerList const& players = map->GetPlayers();
        for (Map::PlayerList::const_iterator it = players.begin(); it != players.end(); ++it)
        {
            Player* member = it->GetSource();
            if (!member || !member->IsAlive() || !RsHalionInTwilight(member))
                continue;
            if (!sPlayerbotsMgr.GetPlayerbotAI(member) || !PlayerbotAI::IsTank(member))
                continue;
            damage = 0;
            return;
        }
    }
};

void AddSC_RubySanctumBotScripts()
{
    new RsHalionRootScript();
    new RsHalionCreatureTrackerScript();
    new RsHalionDarkBreathImmunity();
}
