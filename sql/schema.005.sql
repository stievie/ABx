-- Timestamp when tthe quest was picked up
ALTER TABLE public.player_quests ADD COLUMN picked_up_times bigint NOT NULL DEFAULT 0::bigint;
-- Timestamp when the quest was completed
ALTER TABLE public.player_quests ADD COLUMN completed_time bigint NOT NULL DEFAULT 0::bigint;
ALTER TABLE public.player_quests ADD COLUMN rewarded_time bigint NOT NULL DEFAULT 0::bigint;

UPDATE public.versions SET value = 5 WHERE name = 'schema';
