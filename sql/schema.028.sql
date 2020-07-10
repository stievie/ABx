-- Reducing drop chance of dyes
UPDATE game_item_chances SET chance = 5 where item_uuid IN(SELECT uuid FROM game_items WHERE type = 1001);
UPDATE game_item_chances SET chance = 1 where item_uuid IN(SELECT uuid FROM game_items WHERE type = 1001 AND name IN('Black', 'White'));
UPDATE game_item_chances SET chance = 2 where item_uuid IN(SELECT uuid FROM game_items WHERE type = 1001 AND name IN('Red', 'Pink'));

UPDATE public.versions SET value = 28 WHERE name = 'schema';
