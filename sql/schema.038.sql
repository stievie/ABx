-- Fix bugs. When used with .pak files they must not have leading slashes!

UPDATE public.game_effects SET icon = substring(icon, 2, 100) WHERE icon LIKE '/%';
UPDATE public.versions SET value = value + 1 WHERE name = 'game_effects';
UPDATE public.game_items SET object_file = substring(object_file, 2, 100) WHERE object_file LIKE '/%';
UPDATE public.versions SET value = value + 1 WHERE name = 'game_items';
UPDATE public.game_skills SET icon = substring(icon, 2, 100) WHERE icon LIKE '/%';
UPDATE public.versions SET value = value + 1 WHERE name = 'game_skills';

UPDATE public.versions SET value = 38 WHERE name = 'schema';
