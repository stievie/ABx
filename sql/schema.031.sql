UPDATE public.game_items SET object_file = '/Objects/Item_Stone.xml', icon_file = '/Textures/Icons/Items/Stone.png' WHERE idx = 100004;
UPDATE public.game_items SET object_file = '/Objects/Item_Coal.xml', icon_file = '/Textures/Icons/Items/Coal.png' WHERE idx = 100003;

INSERT INTO public.game_items VALUES ('010c9c23-8cea-42b0-9d9a-7491a00d3172', 511, 'Basic Staff', '/scripts/items/weapons/twohanded/staffs.lua', '/Objects/BasicStuff.xml', '/Textures/Icons/Items/BasicStaff.png', 38, 0, 0, 0, '00000000-0000-0000-0000-000000000000', '', 52);
INSERT INTO public.game_item_chances VALUES (public.random_guid(), '00000000-0000-0000-0000-000000000000', '010c9c23-8cea-42b0-9d9a-7491a00d3172', 100);

UPDATE public.versions SET value = value + 1 WHERE name = 'game_items';

UPDATE public.versions SET value = 31 WHERE name = 'schema';
