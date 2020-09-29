ALTER TABLE public.bans ADD COLUMN hits int NOT NULL DEFAULT 0;

UPDATE public.versions SET value = 41 WHERE name = 'schema';
