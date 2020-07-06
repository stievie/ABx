-- Keep non stacklable items for a certain time

ALTER TABLE public.concrete_items ADD COLUMN sold bigint NOT NULL DEFAULT 0;
CREATE INDEX concrete_items_sold ON concrete_items USING btree (sold);

UPDATE public.versions SET value = 25 WHERE name = 'schema';
