-- Rename Monk to Priest

UPDATE public.game_professions SET name = 'Priest', abbr = 'P'  WHERE uuid = '73156b15-50f4-11e8-a7ca-02100700d6f0';
UPDATE public.game_professions SET abbr = 'M'  WHERE uuid = '85d0939b-50f4-11e8-a7ca-02100700d6f0';
UPDATE public.players SET profession = 'P' WHERE profession = 'Mo';
UPDATE public.players SET profession2 = 'P' WHERE profession2 = 'Mo';
UPDATE public.players SET profession = 'M' WHERE profession = 'Me';
UPDATE public.players SET profession2 = 'M' WHERE profession2 = 'Me';

UPDATE public.versions SET value = value + 1 WHERE name = 'game_professions';

UPDATE public.versions SET value = 13 WHERE name = 'schema';
