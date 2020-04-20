ALTER TABLE public.game_items ADD COLUMN trade_able integer NOT NULL DEFAULT 0;

UPDATE public.game_items SET trade_able = 1 WHERE idx IN(
  500, 501, 502, 503, 504, 505, 506, 507, 508,
  100000, 100001, 100002
);
UPDATE public.versions SET value = value + 1 WHERE name = 'game_items';

UPDATE public.versions SET value = 14 WHERE name = 'schema';
