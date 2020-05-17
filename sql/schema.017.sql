-- Add a nice Axe

INSERT INTO public.game_items VALUES ('30834b3d-b3d5-4eae-ab58-bdb79afe5519', 509, 'Double Headed Axe', '/scripts/items/weapons/leadhand/axes.lua', '/Objects/DoubleHeadedAxe.xml', '/Textures/Icons/Items/DoubleHeadedAxe.png', 30, 0, 0, 0, '00000000-0000-0000-0000-000000000000', '', 4);

UPDATE public.versions SET value = value + 1 WHERE name = 'game_items';

INSERT INTO public.game_item_chances VALUES ('cfe32300-17cb-4373-9ff7-504ad8d90029', 'a13b71f8-fe19-4bf5-bba7-c7642c796c0f', '30834b3d-b3d5-4eae-ab58-bdb79afe5519', 100);

UPDATE public.versions SET value = 17 WHERE name = 'schema';
