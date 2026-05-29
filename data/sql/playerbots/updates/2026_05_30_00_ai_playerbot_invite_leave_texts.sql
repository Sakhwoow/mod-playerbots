-- Bot greeting when invited to group and farewell when leaving
-- text_loc8 = Russian

INSERT INTO `ai_playerbot_texts` (`name`, `text`, `say_type`, `reply_type`, `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`, `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`) VALUES
('hello', 'Hello!', 0, 0, '', '', '', '', '', '', '', 'Привет!'),
('hello', 'Ready to go!', 0, 0, '', '', '', '', '', '', '', 'Готов!'),
('hello', 'At your service.', 0, 0, '', '', '', '', '', '', '', 'К вашим услугам.'),
('goodbye', 'Goodbye!', 0, 0, '', '', '', '', '', '', '', 'До свидания!'),
('goodbye', 'Farewell!', 0, 0, '', '', '', '', '', '', '', 'Прощайте!'),
('goodbye', 'See you around.', 0, 0, '', '', '', '', '', '', '', 'До встречи.');
