/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HoRStrategy.h"

void WotlkDungeonHoRStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("hor lich king gauntlet",
        { NextAction("hor avoid lich king", ACTION_RAID + 9) }));
}
