/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SMTRIGGERS_H
#define PLAYERBOTS_SMTRIGGERS_H

#include "Trigger.h"

enum ScarletMonasteryIDs
{
    NPC_INQUISITOR_WHITEMANE = 3977,
    SPELL_DEEP_SLEEP         = 9256,
};

class SMWhitemanesDeepSleepTrigger : public Trigger
{
public:
    SMWhitemanesDeepSleepTrigger(PlayerbotAI* ai) : Trigger(ai, "sm whitemane deep sleep") {}
    bool IsActive() override;
};

#endif
