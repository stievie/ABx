UPDATE public.game_items SET object_file = '/Objects/Item_Dye_Black.xml', icon_file = '/Textures/Icons/Items/Dye_Black.png' WHERE idx = 100111;
UPDATE public.game_items SET object_file = '/Objects/Item_Dye_White.xml', icon_file = '/Textures/Icons/Items/Dye_White.png' WHERE idx = 100110;
UPDATE public.game_items SET object_file = '/Objects/Item_Dye_Silver.xml', icon_file = '/Textures/Icons/Items/Dye_Silver.png' WHERE idx = 100109;
UPDATE public.game_items SET object_file = '/Objects/Item_Dye_Red.xml', icon_file = '/Textures/Icons/Items/Dye_Red.png' WHERE idx = 100108;
UPDATE public.game_items SET object_file = '/Objects/Item_Dye_Purple.xml', icon_file = '/Textures/Icons/Items/Dye_Purple.png' WHERE idx = 100107;
UPDATE public.game_items SET object_file = '/Objects/Item_Dye_Pink.xml', icon_file = '/Textures/Icons/Items/Dye_Pink.png' WHERE idx = 100106;
UPDATE public.game_items SET object_file = '/Objects/Item_Dye_Orange.xml', icon_file = '/Textures/Icons/Items/Dye_Orange.png' WHERE idx = 100105;
UPDATE public.game_items SET object_file = '/Objects/Item_Dye_Green.xml', icon_file = '/Textures/Icons/Items/Dye_Green.png' WHERE idx = 100104;
UPDATE public.game_items SET object_file = '/Objects/Item_Dye_Grey.xml', icon_file = '/Textures/Icons/Items/Dye_Grey.png' WHERE idx = 100103;
UPDATE public.game_items SET object_file = '/Objects/Item_Dye_Yellow.xml', icon_file = '/Textures/Icons/Items/Dye_Yellow.png' WHERE idx = 100102;
UPDATE public.game_items SET object_file = '/Objects/Item_Dye_Brown.xml', icon_file = '/Textures/Icons/Items/Dye_Brown.png' WHERE idx = 100101;
UPDATE public.game_items SET object_file = '/Objects/Item_Dye_Blue.xml', icon_file = '/Textures/Icons/Items/Dye_Blue.png' WHERE idx = 100100;

UPDATE public.game_items SET schript_file = '/scripts/items/dye.lua' WHERE idx in(100100, 100101, 100102, 100103, 100104, 100105, 100106, 100107, 100108, 100109, 100110, 100111);

UPDATE public.versions SET value = value + 1 WHERE name = 'game_items';

UPDATE public.versions SET value = 35 WHERE name = 'schema';
