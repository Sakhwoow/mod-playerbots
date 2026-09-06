/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TBMACTIONCONTEXT_H
#define PLAYERBOTS_TBMACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "TBMActions.h"

class TbcDungeonBlackMorassActionContext : public NamedObjectContext<Action>
{
public:
    TbcDungeonBlackMorassActionContext() : NamedObjectContext<Action>(false, true)
    {
        creators["tbm attack wave mob"] =
            &TbcDungeonBlackMorassActionContext::tbm_attack_wave_mob;
    }

private:
    static Action* tbm_attack_wave_mob(PlayerbotAI* botAI) { return new TbmAttackWaveMobAction(botAI); }
};

#endif
