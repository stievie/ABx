-- Fix bugs. When used with .pak files they must not have leading slashes!
-- Missed some

UPDATE public.game_items SET icon_file = substring(icon_file, 2, 100) WHERE icon_file LIKE '/%';
UPDATE public.versions SET value = value + 1 WHERE name = 'game_items';

UPDATE public.versions SET value = 39 WHERE name = 'schema';
