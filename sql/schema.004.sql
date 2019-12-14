-- Quest reward
ALTER TABLE public.game_quests ADD COLUMN reward_xp int NOT NULL DEFAULT 0;
ALTER TABLE public.game_quests ADD COLUMN reward_money int NOT NULL DEFAULT 0;
-- Semicolon separated list of item UUIDs
ALTER TABLE public.game_quests ADD COLUMN reward_items text NOT NULL DEFAULT ''::character varying;

UPDATE public.versions SET value = 4 WHERE name = 'schema';
