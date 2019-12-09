-- Auto generate UUID for some tables

CREATE FUNCTION public.random_guid() RETURNS character
    LANGUAGE sql
    AS $$SELECT uuid_in(md5(random()::text || clock_timestamp()::text)::cstring)::text$$;

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

INSERT INTO public.versions (name, value, internal) VALUES ('schema', 1, 1);
