-- Add program version of service

ALTER TABLE public.services ADD COLUMN version bigint NOT NULL DEFAULT 0::bigint;

UPDATE public.versions SET value = 11 WHERE name = 'schema';
