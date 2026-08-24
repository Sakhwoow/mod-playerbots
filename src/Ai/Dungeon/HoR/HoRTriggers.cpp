/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HoRTriggers.h"
#include "AiObjectContext.h"
#include "Playerbots.h"

bool HoRLichKingGauntletTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return target && target->GetEntry() == NPC_LICH_KING_HOR_GAUNTLET;
}
