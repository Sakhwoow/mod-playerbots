/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "TellMasterAction.h"

#include "Event.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"

bool TellMasterAction::Execute(Event /*event*/)
{
    botAI->TellMaster(text);
    return true;
}

bool OutOfReactRangeAction::Execute(Event /*event*/)
{
    botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault("string_wait_for_me", "Wait for me!", {}));
    return true;
}

bool OutOfReactRangeAction::isUseful()
{
    bool canFollow = Follow(AI_VALUE(Unit*, "group leader"));
    if (!canFollow)
    {
        return false;
    }

    return true;
}
