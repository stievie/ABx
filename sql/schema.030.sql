UPDATE public.game_items SET name = 'Black Dye' WHERE idx = 100111;
UPDATE public.game_items SET name = 'White Dye' WHERE idx = 100110;
UPDATE public.game_items SET name = 'Silver Dye' WHERE idx = 100109;
UPDATE public.game_items SET name = 'Red Dye' WHERE idx = 100108;
UPDATE public.game_items SET name = 'Purple Dye' WHERE idx = 100107;
UPDATE public.game_items SET name = 'Pink Dye' WHERE idx = 100106;
UPDATE public.game_items SET name = 'Orange Dye' WHERE idx = 100105;
UPDATE public.game_items SET name = 'Green Dye' WHERE idx = 100104;
UPDATE public.game_items SET name = 'Grey Dye' WHERE idx = 100103;
UPDATE public.game_items SET name = 'Yellow Dye' WHERE idx = 100102;
UPDATE public.game_items SET name = 'Brown Dye' WHERE idx = 100101;
UPDATE public.game_items SET name = 'Blue Dye' WHERE idx = 100100;

UPDATE public.versions SET value = value + 1 WHERE name = 'game_items';

UPDATE public.versions SET value = 30 WHERE name = 'schema';
