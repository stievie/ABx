-- Add some materials
INSERT INTO public.game_items VALUES ('204ba0f2-7008-4d82-878c-a0d90db1e096', 100011, 'Papyrus', '', '/Objects/Item_Papyrus.xml', '/Textures/Icons/Items/Papyrus.png', 1000, 0, 10, 0, '00000000-0000-0000-0000-000000000000', '', 21);
INSERT INTO public.game_item_chances VALUES (public.random_guid(), '00000000-0000-0000-0000-000000000000', '204ba0f2-7008-4d82-878c-a0d90db1e096', 10);

INSERT INTO public.game_items VALUES ('7fcb4b71-ffc6-4680-9ac9-b6c561b87804', 100012, 'Wool', '', '/Objects/Item_Wool.xml', '/Textures/Icons/Items/Wool.png', 1000, 0, 4, 0, '00000000-0000-0000-0000-000000000000', '', 21);
INSERT INTO public.game_item_chances VALUES (public.random_guid(), '00000000-0000-0000-0000-000000000000', '7fcb4b71-ffc6-4680-9ac9-b6c561b87804', 20);

INSERT INTO public.game_items VALUES ('2422864e-5e9c-416d-8ef9-688bd4d6bcd4', 100013, 'Linen', '', '/Objects/Item_Linen.xml', '/Textures/Icons/Items/Linen.png', 1000, 0, 7, 0, '00000000-0000-0000-0000-000000000000', '', 21);
INSERT INTO public.game_item_chances VALUES (public.random_guid(), '00000000-0000-0000-0000-000000000000', '2422864e-5e9c-416d-8ef9-688bd4d6bcd4', 10);

INSERT INTO public.game_items VALUES ('815250df-c046-4567-9f3e-3f4c9db39f3e', 100014, 'Obsidian', '', '/Objects/Item_Obsidian.xml', '/Textures/Icons/Items/Obsidian.png', 1000, 0, 70, 0, '00000000-0000-0000-0000-000000000000', '', 21);
INSERT INTO public.game_item_chances VALUES (public.random_guid(), '00000000-0000-0000-0000-000000000000', '815250df-c046-4567-9f3e-3f4c9db39f3e', 2);

UPDATE public.versions SET value = value + 1 WHERE name = 'game_items';

UPDATE public.versions SET value = 34 WHERE name = 'schema';
