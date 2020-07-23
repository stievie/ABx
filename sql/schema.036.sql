-- Add some materials
INSERT INTO public.game_items VALUES ('6ed4a76d-51f0-4636-860e-3c168a0791e2', 100015, 'Diamons', '', '/Objects/Item_Diamond.xml', '/Textures/Icons/Items/Diamond.png', 1000, 0, 20, 0, '00000000-0000-0000-0000-000000000000', '', 21);
INSERT INTO public.game_item_chances VALUES (public.random_guid(), '00000000-0000-0000-0000-000000000000', '6ed4a76d-51f0-4636-860e-3c168a0791e2', 1);

INSERT INTO public.game_items VALUES ('3e9b88cf-a38d-42a1-a6e4-1c23ceeb274a', 100016, 'Ruby', '', '/Objects/Item_Ruby.xml', '/Textures/Icons/Items/Ruby.png', 1000, 0, 17, 0, '00000000-0000-0000-0000-000000000000', '', 21);
INSERT INTO public.game_item_chances VALUES (public.random_guid(), '00000000-0000-0000-0000-000000000000', '3e9b88cf-a38d-42a1-a6e4-1c23ceeb274a', 2);

INSERT INTO public.game_items VALUES ('5e0746ec-8240-473c-b228-583cdb4b83e7', 100017, 'Saphire', '', '/Objects/Item_Saphire.xml', '/Textures/Icons/Items/Saphire.png', 1000, 0, 17, 0, '00000000-0000-0000-0000-000000000000', '', 21);
INSERT INTO public.game_item_chances VALUES (public.random_guid(), '00000000-0000-0000-0000-000000000000', '5e0746ec-8240-473c-b228-583cdb4b83e7', 2);

INSERT INTO public.game_items VALUES ('240d3704-b38c-41a8-a6db-332a36d4d726', 100018, 'Smaragd', '', '/Objects/Item_Smaragd.xml', '/Textures/Icons/Items/Smaragd.png', 1000, 0, 17, 0, '00000000-0000-0000-0000-000000000000', '', 21);
INSERT INTO public.game_item_chances VALUES (public.random_guid(), '00000000-0000-0000-0000-000000000000', '240d3704-b38c-41a8-a6db-332a36d4d726', 2);

ALTER TABLE public.game_items RENAME schript_file TO script_file;

UPDATE public.versions SET value = value + 1 WHERE name = 'game_items';

UPDATE public.versions SET value = 36 WHERE name = 'schema';
