DELETE FROM `ai_playerbot_texts_chance` WHERE `name` IN (
    'maintenance_started', 'maintenance_alt_bot_error', 'maintenance_random_bot_error',
    'autogear_started', 'autogear_alt_bot_error', 'autogear_random_bot_error'
);
DELETE FROM `ai_playerbot_texts` WHERE `name` IN (
    'maintenance_started', 'maintenance_alt_bot_error', 'maintenance_random_bot_error',
    'autogear_started', 'autogear_alt_bot_error', 'autogear_random_bot_error'
);

INSERT INTO `ai_playerbot_texts`
    (`id`, `name`, `text`, `say_type`, `reply_type`, `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`, `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8`)
VALUES
    (2053, 'maintenance_started',          'I''m maintaining',                              0, 0, '', '', '', '', '', '', '', 'Провожу обслуживание.'),
    (2054, 'maintenance_alt_bot_error',    'You cannot use maintenance on alt bots.',       0, 0, '', '', '', '', '', '', '', 'Обслуживание недоступно для альт-ботов.'),
    (2055, 'maintenance_random_bot_error', 'You cannot use maintenance on random bots.',    0, 0, '', '', '', '', '', '', '', 'Обслуживание недоступно для рандом-ботов.'),
    (2056, 'autogear_started',             'I''m auto gearing',                             0, 0, '', '', '', '', '', '', '', 'Подбираю снаряжение.'),
    (2057, 'autogear_alt_bot_error',       'You cannot use autogear on alt bots.',          0, 0, '', '', '', '', '', '', '', 'Автоэкипировка недоступна для альт-ботов.'),
    (2058, 'autogear_random_bot_error',    'You cannot use autogear on random bots.',       0, 0, '', '', '', '', '', '', '', 'Автоэкипировка недоступна для рандом-ботов.');

INSERT INTO `ai_playerbot_texts_chance` (`name`, `probability`)
VALUES
    ('maintenance_started',          100),
    ('maintenance_alt_bot_error',    100),
    ('maintenance_random_bot_error', 100),
    ('autogear_started',             100),
    ('autogear_alt_bot_error',       100),
    ('autogear_random_bot_error',    100);
