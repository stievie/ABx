CREATE INDEX concrete_items_item_uuid ON concrete_items USING btree (item_uuid);

UPDATE public.versions SET value = 21 WHERE name = 'schema';
