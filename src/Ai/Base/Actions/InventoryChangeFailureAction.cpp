/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "InventoryChangeFailureAction.h"

#include "Event.h"
#include "Playerbots.h"

std::map<InventoryResult, std::string> InventoryChangeFailureAction::messages;

bool InventoryChangeFailureAction::Execute(Event event)
{
    if (!botAI->GetMaster())
        return false;

    if (messages.empty())
    {
        messages[EQUIP_ERR_CANT_EQUIP_LEVEL_I] = "Мой уровень слишком низкий";
        messages[EQUIP_ERR_CANT_EQUIP_SKILL] = "Мой навык слишком низкий";
        messages[EQUIP_ERR_ITEM_DOESNT_GO_TO_SLOT] = "Неверный слот";
        messages[EQUIP_ERR_BAG_FULL] = "Мои сумки полны";
        messages[EQUIP_ERR_NONEMPTY_BAG_OVER_OTHER_BAG] = "Эта сумка не пустая";
        messages[EQUIP_ERR_CANT_TRADE_EQUIP_BAGS] = "Нельзя обменять надетые сумки";
        messages[EQUIP_ERR_ONLY_AMMO_CAN_GO_HERE] = "Неверный слот (только боеприпасы)";
        messages[EQUIP_ERR_NO_REQUIRED_PROFICIENCY] = "У меня нет нужного навыка";
        messages[EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE] = "Нет свободного слота снаряжения";
        messages[EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM] = "Я никогда не смогу использовать это";
        messages[EQUIP_ERR_YOU_CAN_NEVER_USE_THAT_ITEM2] = "Я никогда не смогу использовать это";
        messages[EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE2] = messages[EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE];
        messages[EQUIP_ERR_CANT_EQUIP_WITH_TWOHANDED] = "Нельзя надеть с двуручным оружием";
        messages[EQUIP_ERR_CANT_DUAL_WIELD] = "Я не умею сражаться двумя оружиями";
        messages[EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG] = "Этот предмет не подходит для этой сумки";
        messages[EQUIP_ERR_ITEM_DOESNT_GO_INTO_BAG2] = "Этот предмет не подходит для этой сумки";
        messages[EQUIP_ERR_CANT_CARRY_MORE_OF_THIS] = "Больше таких предметов не унести";
        messages[EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE3] = messages[EQUIP_ERR_NO_EQUIPMENT_SLOT_AVAILABLE];
        messages[EQUIP_ERR_ITEM_CANT_STACK] = "Предмет нельзя складывать в стопку";
        messages[EQUIP_ERR_ITEM_CANT_BE_EQUIPPED] = "Предмет нельзя надеть";
        messages[EQUIP_ERR_ITEMS_CANT_BE_SWAPPED] = "Эти предметы нельзя поменять местами";
        messages[EQUIP_ERR_SLOT_IS_EMPTY] = "Нечего надевать";
        messages[EQUIP_ERR_ITEM_NOT_FOUND] = "Предмет не найден";
        messages[EQUIP_ERR_CANT_DROP_SOULBOUND] = "Нельзя выбросить душесвязанный предмет";
        messages[EQUIP_ERR_OUT_OF_RANGE] = "Слишком далеко";
        messages[EQUIP_ERR_TRIED_TO_SPLIT_MORE_THAN_COUNT] = "Неверное количество для разделения";
        messages[EQUIP_ERR_COULDNT_SPLIT_ITEMS] = "Нельзя разделить этот предмет";
        messages[EQUIP_ERR_MISSING_REAGENT] = "Не хватает реагентов";
        messages[EQUIP_ERR_NOT_ENOUGH_MONEY] = "Недостаточно денег";
        messages[EQUIP_ERR_NOT_A_BAG] = "Это не сумка";
        messages[EQUIP_ERR_CAN_ONLY_DO_WITH_EMPTY_BAGS] = "Сумка не пустая";
        messages[EQUIP_ERR_DONT_OWN_THAT_ITEM] = "Это не мой предмет";
        messages[EQUIP_ERR_CAN_EQUIP_ONLY1_QUIVER] = "Можно надеть только один колчан";
        messages[EQUIP_ERR_MUST_PURCHASE_THAT_BAG_SLOT] = "Нужно сначала купить этот слот";
        messages[EQUIP_ERR_TOO_FAR_AWAY_FROM_BANK] = "Я слишком далеко от банка";
        messages[EQUIP_ERR_ITEM_LOCKED] = "Предмет заблокирован";
        messages[EQUIP_ERR_YOU_ARE_STUNNED] = "Я оглушён";
        messages[EQUIP_ERR_YOU_ARE_DEAD] = "Я мёртв";
        messages[EQUIP_ERR_CANT_DO_RIGHT_NOW] = "Сейчас не могу этого сделать";
        messages[EQUIP_ERR_INT_BAG_ERROR] = "Внутренняя ошибка";
        messages[EQUIP_ERR_CAN_EQUIP_ONLY1_BOLT] = "Можно использовать только болты";
        messages[EQUIP_ERR_CAN_EQUIP_ONLY1_AMMOPOUCH] = "Можно использовать только патронташ";
        messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED] = "Предмет нельзя завернуть";
        messages[EQUIP_ERR_EQUIPPED_CANT_BE_WRAPPED] = messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED];
        messages[EQUIP_ERR_WRAPPED_CANT_BE_WRAPPED] = messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED];
        messages[EQUIP_ERR_BOUND_CANT_BE_WRAPPED] = messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED];
        messages[EQUIP_ERR_UNIQUE_CANT_BE_WRAPPED] = messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED];
        messages[EQUIP_ERR_BAGS_CANT_BE_WRAPPED] = messages[EQUIP_ERR_STACKABLE_CANT_BE_WRAPPED];
        messages[EQUIP_ERR_ALREADY_LOOTED] = "Уже подобрано";
        messages[EQUIP_ERR_INVENTORY_FULL] = "Мой инвентарь полон";
        messages[EQUIP_ERR_BANK_FULL] = "Мой банк полон";
        messages[EQUIP_ERR_ITEM_IS_CURRENTLY_SOLD_OUT] = "Товар распродан";
        messages[EQUIP_ERR_BAG_FULL3] = messages[EQUIP_ERR_BANK_FULL];
        messages[EQUIP_ERR_ITEM_NOT_FOUND2] = messages[EQUIP_ERR_ITEM_NOT_FOUND];
        messages[EQUIP_ERR_ITEM_CANT_STACK2] = messages[EQUIP_ERR_ITEM_CANT_STACK];
        messages[EQUIP_ERR_BAG_FULL4] = messages[EQUIP_ERR_BAG_FULL];
        messages[EQUIP_ERR_ITEM_SOLD_OUT] = messages[EQUIP_ERR_ITEM_IS_CURRENTLY_SOLD_OUT];
        messages[EQUIP_ERR_OBJECT_IS_BUSY] = "Объект занят";
        messages[EQUIP_ERR_NOT_IN_COMBAT] = "Я в бою";
        messages[EQUIP_ERR_NOT_WHILE_DISARMED] = "Нельзя без оружия";
        messages[EQUIP_ERR_BAG_FULL6] = messages[EQUIP_ERR_BAG_FULL];
        messages[EQUIP_ERR_CANT_EQUIP_RANK] = "Недостаточно звания";
        messages[EQUIP_ERR_CANT_EQUIP_REPUTATION] = "Недостаточно репутации";
        messages[EQUIP_ERR_TOO_MANY_SPECIAL_BAGS] = "Слишком много специальных сумок";
        messages[EQUIP_ERR_LOOT_CANT_LOOT_THAT_NOW] = "Сейчас нельзя подобрать это";
    }

    WorldPacket p(event.getPacket());
    p.rpos(0);
    uint8 err;
    p >> err;
    if (err == EQUIP_ERR_OK)
        return false;

    std::string const msg = messages[(InventoryResult)err];
    if (!msg.empty())
    {
        botAI->TellError(msg);
        return true;
    }

    return false;
}
