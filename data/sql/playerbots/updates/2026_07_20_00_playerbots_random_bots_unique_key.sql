-- Add UNIQUE constraint on (owner, bot, event) to playerbots_random_bots.
-- Required for INSERT ... ON DUPLICATE KEY UPDATE to work atomically,
-- eliminating the race condition that caused [1062] / [1213] deadlock crashes.
-- Deduplicate existing rows first (keep latest id per key).

DELETE rb FROM playerbots_random_bots rb
INNER JOIN (
    SELECT MAX(id) AS keep_id, owner, bot, event
    FROM playerbots_random_bots
    GROUP BY owner, bot, event
    HAVING COUNT(*) > 1
) dup ON rb.owner = dup.owner AND rb.bot = dup.bot AND rb.event = dup.event AND rb.id != dup.keep_id;

SET @index_exists := (
    SELECT COUNT(1)
    FROM INFORMATION_SCHEMA.STATISTICS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'playerbots_random_bots'
      AND INDEX_NAME = 'idx_owner_bot_event'
);

SET @ddl := IF(@index_exists > 0,
    'ALTER TABLE `playerbots_random_bots` DROP INDEX `idx_owner_bot_event`',
    'SELECT 1'
);
PREPARE stmt FROM @ddl;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;

SET @unique_exists := (
    SELECT COUNT(1)
    FROM INFORMATION_SCHEMA.STATISTICS
    WHERE TABLE_SCHEMA = DATABASE()
      AND TABLE_NAME = 'playerbots_random_bots'
      AND INDEX_NAME = 'uq_owner_bot_event'
      AND NON_UNIQUE = 0
);

SET @ddl := IF(@unique_exists = 0,
    'ALTER TABLE `playerbots_random_bots` ADD UNIQUE KEY `uq_owner_bot_event` (`owner`, `bot`, `event`)',
    'SELECT "UNIQUE KEY uq_owner_bot_event already exists."'
);
PREPARE stmt FROM @ddl;
EXECUTE stmt;
DEALLOCATE PREPARE stmt;
