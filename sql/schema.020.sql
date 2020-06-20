ALTER TABLE public.concrete_items ADD COLUMN flags integer NOT NULL DEFAULT 0;

UPDATE public.versions SET value = 20 WHERE name = 'schema';
