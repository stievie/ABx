-- Auto generate UUID for some tables

ALTER TABLE public.game_attributes ALTER COLUMN uuid SET DEFAULT public.random_guid();
ALTER TABLE public.game_effects ALTER COLUMN uuid SET DEFAULT public.random_guid();
ALTER TABLE public.game_item_chances ALTER COLUMN uuid SET DEFAULT public.random_guid();
ALTER TABLE public.game_items ALTER COLUMN uuid SET DEFAULT public.random_guid();
ALTER TABLE public.game_maps ALTER COLUMN uuid SET DEFAULT public.random_guid();
ALTER TABLE public.game_music ALTER COLUMN uuid SET DEFAULT public.random_guid();
ALTER TABLE public.game_professions ALTER COLUMN uuid SET DEFAULT public.random_guid();
ALTER TABLE public.game_quests ALTER COLUMN uuid SET DEFAULT public.random_guid();
ALTER TABLE public.game_skills ALTER COLUMN uuid SET DEFAULT public.random_guid();
ALTER TABLE public.versions ALTER COLUMN uuid SET DEFAULT public.random_guid();

UPDATE public.versions SET value = 1 WHERE name = 'schema';
