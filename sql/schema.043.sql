-- Fix typo

UPDATE public.game_items SET name = 'Diamond' WHERE idx = 100015;
UPDATE public.versions SET value = value + 1 WHERE name = 'game_items';

UPDATE public.versions SET value = 43 WHERE name = 'schema';
