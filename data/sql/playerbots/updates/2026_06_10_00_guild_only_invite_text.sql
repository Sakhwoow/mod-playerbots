DELETE FROM `ai_playerbot_texts` WHERE `name` = 'guild_only_invite';
INSERT INTO `ai_playerbot_texts` (`name`, `text`, `say_type`, `reply_type`, `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`, `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`) VALUES
    ('guild_only_invite', 'Sorry, I only accept party invitations from my guildmates.', 0, 0, '', '', '', '', '', '', '', 'Извини, я принимаю приглашения в группу только от согильдийцев.');
