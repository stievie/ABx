-- Store skill stats in DB

ALTER TABLE public.game_skills ADD COLUMN activation bigint NOT NULL DEFAULT 0::bigint;
ALTER TABLE public.game_skills ADD COLUMN recharge bigint NOT NULL DEFAULT 0::bigint;
ALTER TABLE public.game_skills ADD COLUMN const_energy bigint NOT NULL DEFAULT 0::bigint;
ALTER TABLE public.game_skills ADD COLUMN const_energy_regen bigint NOT NULL DEFAULT 0::bigint;
ALTER TABLE public.game_skills ADD COLUMN const_adrenaline bigint NOT NULL DEFAULT 0::bigint;
ALTER TABLE public.game_skills ADD COLUMN const_overcast bigint NOT NULL DEFAULT 0::bigint;
ALTER TABLE public.game_skills ADD COLUMN const_hp bigint NOT NULL DEFAULT 0::bigint;

UPDATE public.versions SET value = value + 1 WHERE name = 'game_skills';

UPDATE public.versions SET value = 10 WHERE name = 'schema';
