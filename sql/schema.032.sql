UPDATE public.game_items SET object_file = '/Objects/Item_Leather.xml', icon_file = '/Textures/Icons/Items/Leather.png' WHERE idx = 100006;
UPDATE public.game_items SET object_file = '/Objects/Item_BoldOfCloth.xml', icon_file = '/Textures/Icons/Items/BoldOfCloth.png' WHERE idx = 100005;

-- Make some items salvageable
UPDATE public.game_items SET item_flags = item_flags | 64 WHERE type IN(
  10, 11, 12, 13, 14,
  30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 44
);

UPDATE public.versions SET value = value + 1 WHERE name = 'game_items';

UPDATE public.versions SET value = 32 WHERE name = 'schema';
