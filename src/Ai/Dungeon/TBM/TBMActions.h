/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TBMACTIONS_H
#define PLAYERBOTS_TBMACTIONS_H

#include "AttackAction.h"

class TbmAttackWaveMobAction : public AttackAction
{
public:
    TbmAttackWaveMobAction(PlayerbotAI* botAI) : AttackAction(botAI, "tbm attack wave mob") {}

    bool Execute(Event event) override;
};

#endif
