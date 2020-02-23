-- Timestamp when the item was deleted up. If noot deleted this is 0.
ALTER TABLE public.concrete_items ADD COLUMN deleted bigint NOT NULL DEFAULT 0::bigint;

CREATE INDEX concrete_items_deleted ON concrete_items USING btree (deleted);

UPDATE public.versions SET value = 6 WHERE name = 'schema';
