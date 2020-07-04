-- Add a two handed staff

INSERT INTO public.game_items VALUES ('5664d215-363f-4182-aa05-a27a39d55309', 510, 'Sun Staff', '/scripts/items/weapons/twohanded/staffs.lua', '/Objects/SunStuff.xml', '/Textures/Icons/Items/SunStaff.png', 38, 0, 0, 0, '00000000-0000-0000-0000-000000000000', '', 20);

UPDATE public.versions SET value = value + 1 WHERE name = 'game_items';

INSERT INTO public.game_item_chances VALUES ('b2968f5a-617b-47e6-8539-764d5c50233f', 'a13b71f8-fe19-4bf5-bba7-c7642c796c0f', '5664d215-363f-4182-aa05-a27a39d55309', 50);

UPDATE public.versions SET value = 24 WHERE name = 'schema';
