/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_SMACTIONCONTEXT_H
#define PLAYERBOTS_SMACTIONCONTEXT_H

#include "NamedObjectContext.h"
#include "SMActions.h"

class VanillaDungeonSMActionContext : public NamedObjectContext<Action>
{
public:
    VanillaDungeonSMActionContext()
    {
        creators["sm stop attacking whitemane"] = &VanillaDungeonSMActionContext::sm_stop_attacking_whitemane;
    }

private:
    static Action* sm_stop_attacking_whitemane(PlayerbotAI* ai) { return new SMStopAttackingWhitemaneAction(ai); }
};

#endif
