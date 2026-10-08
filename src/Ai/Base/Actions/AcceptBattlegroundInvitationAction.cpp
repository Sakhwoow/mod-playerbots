/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "AcceptBattlegroundInvitationAction.h"
#include "Event.h"
#include "GroupMgr.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

bool AcceptBgInvitationAction::Execute(Event /*event*/)
{
    // A bot can queue for a BG while ungrouped (BattleGroundJoinAction.cpp's shouldJoinBg only
    // lets a group's leader queue, so a regular member never submits a queue request while
    // grouped) and only get invited to port in much later, once the queue actually pops. If a
    // real player grouped this bot (e.g. invited it into a raid) in the meantime, porting in
    // here would silently pull it out of that raid with no way back without the player noticing
    // - same bug class BattleGroundJoinAction.cpp's GroupHasRealPlayerForBG already guards
    // against at queue time, just needed again here since joining the queue and the queue
    // popping can be arbitrarily far apart.
    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
        {
            Player* groupMember = ref->GetSource();
            if (groupMember && (!GET_PLAYERBOT_AI(groupMember) || IsSelfBot(groupMember)))
                return false;
        }
    }

    uint8 type = 0;                      // arenatype if arena
    uint8 unk2 = 0;                      // unk, can be 0x0 (may be if was invited?) and 0x1
    uint32 bgTypeId_ = BATTLEGROUND_WS;  // type id from dbc
    uint16 unk = 0x1F90;                 // 0x1F90 constant?*/
    uint8 action = 1;

    WorldPacket packet(CMSG_BATTLEFIELD_PORT, 20);
    packet << type << unk2 << (uint32)bgTypeId_ << unk << action;
    bot->GetSession()->HandleBattleFieldPortOpcode(packet);

    botAI->ResetStrategies();

    return true;
}
