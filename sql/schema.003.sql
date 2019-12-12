-- Quests

-- Wheter the quest is repeatable
ALTER TABLE public.game_quests ADD COLUMN repeatable int NOT NULL DEFAULT 0;
ALTER TABLE public.game_quests ADD COLUMN description text NOT NULL DEFAULT ''::character varying;
INSERT INTO public.versions (name, value, internal) VALUES ('game_quests', 1, 0);

-- Open quests by player
CREATE TABLE public.player_quests (
    uuid character(36) NOT NULL,
    quests_uuid character(36) NOT NULL,
    player_uuid character(36) NOT NULL,
    completed int NOT NULL DEFAULT 0,
    rewarded int NOT NULL DEFAULT 0,
    progress text
);
ALTER TABLE ONLY public.player_quests ADD CONSTRAINT player_quests_pkey PRIMARY KEY (uuid);
CREATE INDEX player_quests_quests_uuid ON public.player_quests USING btree (quests_uuid);
CREATE INDEX player_quests_player_uuid ON public.player_quests USING btree (player_uuid);
CREATE INDEX player_quests_rewarded ON public.player_quests USING btree (rewarded);

UPDATE public.versions SET value = 3 WHERE name = 'schema';
