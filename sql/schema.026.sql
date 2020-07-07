-- Add craftable item flag

-- Make all weapons, armors, upgrades craftable
UPDATE public.game_items SET item_flags = item_flags | 32 WHERE type IN(
  10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
  30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 44
);

UPDATE public.versions SET value = value + 1 WHERE name = 'game_items';

UPDATE public.versions SET value = 26 WHERE name = 'schema';
