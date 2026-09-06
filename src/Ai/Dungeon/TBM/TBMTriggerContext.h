/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TBMTRIGGERCONTEXT_H
#define PLAYERBOTS_TBMTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "TBMTriggers.h"

class TbcDungeonBlackMorassTriggerContext : public NamedObjectContext<Trigger>
{
public:
    TbcDungeonBlackMorassTriggerContext()
    {
        creators["tbm wave mob nearby"] =
            &TbcDungeonBlackMorassTriggerContext::tbm_wave_mob_nearby;
    }

private:
    static Trigger* tbm_wave_mob_nearby(PlayerbotAI* botAI) { return new TbmWaveMobNearbyTrigger(botAI); }
};

#endif
