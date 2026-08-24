/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HORTRIGGERS_H
#define PLAYERBOTS_HORTRIGGERS_H

#include "Trigger.h"

enum HallsOfReflectionIDs
{
    NPC_LICH_KING_HOR_GAUNTLET = 37226,
};

class HoRLichKingGauntletTrigger : public Trigger
{
public:
    HoRLichKingGauntletTrigger(PlayerbotAI* ai) : Trigger(ai, "hor lich king gauntlet") {}
    bool IsActive() override;
};

#endif
