-- Change the skills lock field to an access field

--enum SkillAccess
--{
--    SkillAccessNone = 0,
--    SkillAccessPlayer = 1,
--    SkillAccessGM = 1 << 1,
--    SkillAccessMonster = 1 << 2
--};

ALTER TABLE public.game_skills ADD COLUMN access bigint NOT NULL DEFAULT 0::bigint;
UPDATE public.game_skills SET access = 2 WHERE is_locked = 1;
UPDATE public.game_skills SET access = 1 WHERE access = 0;
-- None skill
UPDATE public.game_skills SET access = 0 WHERE uuid = '153eea82-50fa-11e8-a7ca-02100700d6f0';
-- Poison Spout
UPDATE public.game_skills SET access = 4 WHERE uuid = '339fb8f3-8bcd-48b5-bb87-2fc2635e60f7';

ALTER TABLE public.game_skills DROP COLUMN is_locked;

UPDATE public.versions SET value = value + 1 WHERE name = 'game_skills';

UPDATE public.versions SET value = 9 WHERE name = 'schema';
