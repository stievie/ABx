ALTER TABLE public.game_items ADD COLUMN item_flags integer NOT NULL DEFAULT 0;

UPDATE public.game_items SET item_flags = item_flags | 1 WHERE stack_able = 1;
UPDATE public.game_items SET item_flags = item_flags | 4 WHERE trade_able = 1;

ALTER TABLE public.game_items DROP COLUMN stack_able;
ALTER TABLE public.game_items DROP COLUMN trade_able;

UPDATE public.versions SET value = value + 1 WHERE name = 'game_items';

UPDATE public.versions SET value = 15 WHERE name = 'schema';
