/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TBMTRIGGERS_H
#define PLAYERBOTS_TBMTRIGGERS_H

#include "Trigger.h"

class TbmWaveMobNearbyTrigger : public Trigger
{
public:
    TbmWaveMobNearbyTrigger(PlayerbotAI* botAI) : Trigger(botAI, "tbm wave mob nearby") {}

    bool IsActive() override;
};

#endif
