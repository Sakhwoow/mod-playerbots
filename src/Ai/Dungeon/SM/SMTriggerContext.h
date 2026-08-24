/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SMTRIGGERCONTEXT_H
#define PLAYERBOTS_SMTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "SMTriggers.h"

class VanillaDungeonSMTriggerContext : public NamedObjectContext<Trigger>
{
public:
    VanillaDungeonSMTriggerContext()
    {
        creators["sm whitemane deep sleep"] = &VanillaDungeonSMTriggerContext::sm_whitemane_deep_sleep;
    }

private:
    static Trigger* sm_whitemane_deep_sleep(PlayerbotAI* ai) { return new SMWhitemanesDeepSleepTrigger(ai); }
};

#endif
