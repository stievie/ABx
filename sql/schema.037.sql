-- Add some materials
INSERT INTO public.game_items VALUES ('3aa9e32d-16ba-4963-928c-18bc0b60ebbf', 100019, 'Glittering Dust', '', '/Objects/Item_GlitteringDust.xml', '/Textures/Icons/Items/GlitteringDust.png', 1000, 0, 4, 0, '00000000-0000-0000-0000-000000000000', '', 21);
INSERT INTO public.game_item_chances VALUES (public.random_guid(), '00000000-0000-0000-0000-000000000000', '3aa9e32d-16ba-4963-928c-18bc0b60ebbf', 100);

UPDATE public.game_items SET object_file = '/Objects/LongBow.xml', icon_file = '/Textures/Icons/Items/LongBow.png' WHERE idx = 507;
INSERT INTO public.game_item_chances VALUES (public.random_guid(), '00000000-0000-0000-0000-000000000000', '5a340ae9-59cf-43c3-b8a1-4477d5c857c7', 100);

UPDATE public.versions SET value = value + 1 WHERE name = 'game_items';

UPDATE public.versions SET value = 37 WHERE name = 'schema';
