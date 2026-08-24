/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HORACTIONCONTEXT_H
#define PLAYERBOTS_HORACTIONCONTEXT_H

#include "HoRActions.h"
#include "NamedObjectContext.h"

class WotlkDungeonHoRActionContext : public NamedObjectContext<Action>
{
public:
    WotlkDungeonHoRActionContext()
    {
        creators["hor avoid lich king"] = &WotlkDungeonHoRActionContext::hor_avoid_lich_king;
    }

private:
    static Action* hor_avoid_lich_king(PlayerbotAI* ai) { return new HoRAvoidLichKingAction(ai); }
};

#endif
