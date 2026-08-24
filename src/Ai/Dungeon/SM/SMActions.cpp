/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "SMActions.h"
#include "AiObjectContext.h"
#include "Pet.h"

bool SMStopAttackingWhitemaneAction::Execute(Event /*event*/)
{
    context->GetValue<Unit*>("current target")->Set(nullptr);
    bot->SetTarget(ObjectGuid::Empty);
    bot->AttackStop();

    // Pets are not affected by Deep Sleep, so explicitly stop the pet too.
    if (Pet* pet = bot->GetPet())
        pet->AttackStop();

    return true;
}
