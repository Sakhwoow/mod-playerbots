/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TBMTriggers.h"
#include "AiObjectContext.h"
#include "Playerbots.h"

bool TbmWaveMobNearbyTrigger::IsActive()
{
    if (bot->IsInCombat())
        return false;

    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    for (ObjectGuid const& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && unit->IsAlive() && unit->ToCreature() && bot->IsHostileTo(unit))
            return true;
    }

    return false;
}
