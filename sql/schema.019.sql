-- For now make all items resell able which are trade able
-- See Include/AB/Entities/Item.h for ItemFlag's

UPDATE public.game_items SET item_flags = item_flags | 16 WHERE item_flags & 4 = 4;

UPDATE public.versions SET value = value + 1 WHERE name = 'game_items';

UPDATE public.versions SET value = 19 WHERE name = 'schema';
