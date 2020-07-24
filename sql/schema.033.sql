INSERT INTO public.game_items VALUES ('2ab6adad-b250-4696-b577-8f2bf68240d1', 100112, 'Salvage Kit', '/scripts/items/salvage_kit.lua', '/Objects/Item_SlavageKit.xml', '/Textures/Icons/Items/SalvageKit.png', 1002, 0, 1000, 0, '00000000-0000-0000-0000-000000000000', '', 4 | 32);

UPDATE public.versions SET value = value + 1 WHERE name = 'game_items';

UPDATE public.versions SET value = 33 WHERE name = 'schema';
