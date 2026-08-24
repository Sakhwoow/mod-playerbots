/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HORACTIONS_H
#define PLAYERBOTS_HORACTIONS_H

#include "Action.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

class HoRAvoidLichKingAction : public Action
{
public:
    HoRAvoidLichKingAction(PlayerbotAI* ai) : Action(ai, "hor avoid lich king") {}
    bool Execute(Event event) override;
};

#endif
