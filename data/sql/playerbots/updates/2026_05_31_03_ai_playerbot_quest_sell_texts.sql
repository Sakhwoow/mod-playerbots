-- Localized quest status, sell and trainer messages
INSERT INTO `ai_playerbot_texts` (`name`, `text`, `say_type`, `reply_type`, `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`, `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`) VALUES
('string_quest',             'Quest',         0, 0, '', 'Quête',       'Quest',       '任务',   '', 'Misión',      '', 'Задание'),
('string_available',         'Available',     0, 0, '', 'Disponible',  'Verfügbar',   '可接取', '', 'Disponible',  '', 'Доступно'),
('string_incompleted',       'Incompleted',   0, 0, '', 'Incomplet',   'Unvollständig','未完成','', 'Incompleto',  '', 'Не выполнено'),
('string_quest_failed',      'Failed',        0, 0, '', 'Échoué',      'Fehlgeschlagen','失败', '', 'Fallido',     '', 'Провалено'),
('string_selling',           'Selling',       0, 0, '', 'Vente de',    'Verkaufe',    '出售',   '', 'Vendiendo',   '', 'Продаю'),
('string_im_maintaining',    'I\'m maintaining', 0, 0, '', 'Je me prépare.', 'Ich bin am Ausrüsten.', '我在维护中。', '', 'Estoy manteniendo.', '', 'Обслуживаюсь.'),
('string_im_auto_gearing',   'I\'m auto gearing', 0, 0, '', 'Je m\'équipe automatiquement.', 'Ich rüste mich automatisch aus.', '我在自动装备中。', '', 'Me estoy equipando automáticamente.', '', 'Автоматически экипируюсь.');
