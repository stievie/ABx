ALTER TABLE public.players ADD COLUMN death_stats text NOT NULL DEFAULT ''::text;

UPDATE public.versions SET value = 16 WHERE name = 'schema';
