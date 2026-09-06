/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TBMActions.h"
#include "AiObjectContext.h"
#include "Playerbots.h"

bool TbmAttackWaveMobAction::Execute(Event /*event*/)
{
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");
    Unit* nearest = nullptr;
    float minDist = 60.0f;

    for (ObjectGuid const& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive() || !unit->ToCreature())
            continue;

        if (!bot->IsHostileTo(unit))
            continue;

        float d = bot->GetDistance(unit);
        if (d < minDist)
        {
            minDist = d;
            nearest = unit;
        }
    }

    if (!nearest)
        return false;

    return Attack(nearest);
}
