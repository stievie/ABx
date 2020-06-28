UPDATE public.game_items SET icon_file = '/Textures/Icons/Items/SurvivorInsignia.png' WHERE uuid = 'e79bc08f-0052-4ee3-ac79-51edcc454a48';
UPDATE public.versions SET value = value + 1 WHERE name = 'game_items';

UPDATE public.versions SET value = 23 WHERE name = 'schema';
