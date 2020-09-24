CREATE INDEX instances_is_running ON public.instances USING btree (is_running);
ALTER TABLE public.instances ADD COLUMN players int NOT NULL DEFAULT 0;

UPDATE public.versions SET value = 40 WHERE name = 'schema';
