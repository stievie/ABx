-- Remove duplicate drop chances. No need to have multiple drop chances when the chance applies to all maps
DELETE FROM public.game_item_chances WHERE map_uuid = '00000000-0000-0000-0000-000000000000' AND uuid NOT IN (SELECT max(uuid) FROM game_item_chances GROUP BY item_uuid);

-- Add some materials
INSERT INTO public.game_items VALUES ('ae7534e3-bae2-4d34-9e87-230564ecb240', 100007, 'Gold', '', '/Objects/Gold.xml', '/Textures/Icons/Items/Gold.png', 1000, 0, 10, 0, '00000000-0000-0000-0000-000000000000', '', 21);
INSERT INTO public.game_item_chances VALUES (public.random_guid(), '00000000-0000-0000-0000-000000000000', 'ae7534e3-bae2-4d34-9e87-230564ecb240', 20);

INSERT INTO public.game_items VALUES ('5a6caacc-366a-4764-9dc0-d3d993947445', 100008, 'Silver', '', '/Objects/Silver.xml', '/Textures/Icons/Items/Silver.png', 1000, 0, 8, 0, '00000000-0000-0000-0000-000000000000', '', 21);
INSERT INTO public.game_item_chances VALUES (public.random_guid(), '00000000-0000-0000-0000-000000000000', '5a6caacc-366a-4764-9dc0-d3d993947445', 25);

INSERT INTO public.game_items VALUES ('36b300e6-1990-4b43-8c47-304bbac173c6', 100009, 'Bronce', '', '/Objects/Bronce.xml', '/Textures/Icons/Items/Bronce.png', 1000, 0, 7, 0, '00000000-0000-0000-0000-000000000000', '', 21);
INSERT INTO public.game_item_chances VALUES (public.random_guid(), '00000000-0000-0000-0000-000000000000', '36b300e6-1990-4b43-8c47-304bbac173c6', 30);

INSERT INTO public.game_items VALUES ('adc32615-aadd-4d61-95d6-e651edde9934', 100010, 'Brass', '', '/Objects/Brass.xml', '/Textures/Icons/Items/Brass.png', 1000, 0, 5, 0, '00000000-0000-0000-0000-000000000000', '', 21);
INSERT INTO public.game_item_chances VALUES (public.random_guid(), '00000000-0000-0000-0000-000000000000', 'adc32615-aadd-4d61-95d6-e651edde9934', 30);

UPDATE public.versions SET value = value + 1 WHERE name = 'game_items';

UPDATE public.versions SET value = 29 WHERE name = 'schema';
