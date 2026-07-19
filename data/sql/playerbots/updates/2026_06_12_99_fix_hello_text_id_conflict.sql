-- Move 'hello' text entry from ID 1900 to 2052 to free ID 1900 for upstream use.
-- The upstream file 2026_06_13_00_ai_playerbot_missing_group_buff_reagent.sql
-- assigns ID 1900 to 'missing_group_buff_reagent', conflicting with this entry.

UPDATE ai_playerbot_texts
SET id = 2052
WHERE id = 1900 AND name = 'hello';
