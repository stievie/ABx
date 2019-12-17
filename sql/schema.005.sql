-- Timestamp when the quest was picked up
ALTER TABLE public.player_quests ADD COLUMN picked_up_times bigint NOT NULL DEFAULT 0::bigint;
-- Timestamp when the quest was completed
ALTER TABLE public.player_quests ADD COLUMN completed_time bigint NOT NULL DEFAULT 0::bigint;
ALTER TABLE public.player_quests ADD COLUMN rewarded_time bigint NOT NULL DEFAULT 0::bigint;
ALTER TABLE public.player_quests ADD COLUMN deleted int NOT NULL DEFAULT 0;
CREATE INDEX player_quests_deleted ON public.player_quests USING btree (deleted);

-- For Quest chains
ALTER TABLE public.game_quests ADD COLUMN depends_on_uuid character(36) NOT NULL DEFAULT '00000000-0000-0000-0000-000000000000'::bpchar;
UPDATE public.versions SET value = value + 1 WHERE name = 'game_quests';

UPDATE public.versions SET value = 5 WHERE name = 'schema';
