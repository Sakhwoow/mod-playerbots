/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license, you may redistribute it
 * and/or modify it under version 3 of the License, or (at your option), any later version.
 */

#include "HelpAction.h"

#include "ChatActionContext.h"
#include "Event.h"
#include "AiObjectContext.h"

HelpAction::HelpAction(PlayerbotAI* botAI) : Action(botAI, "help") { chatContext = new ChatActionContext(); }

HelpAction::~HelpAction() { delete chatContext; }

bool HelpAction::Execute(Event /*event*/)
{
    TellChatCommands();
    TellStrategies();
    return true;
}

void HelpAction::TellChatCommands()
{
    botAI->TellError("=== Команды (пишите в личку на английском) ===");

    botAI->TellError("Инфо: stats · spells · quests · reputation · aura · who · tell target · tell pvp stats · emblems · los · position");

    botAI->TellError("Бой: co ? (боевые стратегии) · nc ? (небоевые) · de ? (смерть) · buff · summon · formation [тип] · stance [тип] · rti");

    botAI->TellError("Предметы: equip [предмет] · unequip [предмет] · sell [предмет] · buy [предмет] · use [предмет] · trade [предмет] · destroy [предмет] · item count [предмет]");

    botAI->TellError("Экипировка: autogear · autogear bis · equip upgrade · glyphs · glyph equip · repair · maintenance · trainer");

    botAI->TellError("Квесты: share [квест] · drop [квест] · reward [квест] · quests · query quest [квест] · clean quest log · gossip hello");

    botAI->TellError("Перемещение: teleport · taxi · go · home · summon · release · repop · spirit healer");

    botAI->TellError("Банк/Почта: bank · gb (банк гильдии) · mail · sendmail · sell (продать всё серое)");

    botAI->TellError("Питомец: pet · pet attack · tame");

    botAI->TellError("Прочее: help · wipe · range · save mana · ll (лут) · buff · invite · leave · join · lfg · roll · calc · wts · hire · craft · flag · cheat");

    botAI->TellError("Также принимает ссылки: [предмет], [квест], [объект]");
}

void HelpAction::TellStrategies()
{
    std::ostringstream out;
    out << "Доступные стратегии (команды co/nc/de): ";
    out << CombineSupported(botAI->GetAiObjectContext()->GetSupportedStrategies());
    botAI->TellError(out.str());
}

std::string const HelpAction::CombineSupported(std::set<std::string> commands)
{
    std::ostringstream out;

    for (std::set<std::string>::iterator i = commands.begin(); i != commands.end();)
    {
        out << *i;
        if (++i != commands.end())
            out << ", ";
    }

    return out.str();
}
