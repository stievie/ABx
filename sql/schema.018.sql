-- Add can_drop field
-- There may be times when an Item does not drop, e.g. a Toilet paper only drops
-- during some pandemic, or something...

ALTER TABLE public.game_item_chances ADD COLUMN can_drop integer NOT NULL DEFAULT 1;

UPDATE public.versions SET value = 18 WHERE name = 'schema';
