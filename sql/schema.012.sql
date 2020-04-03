-- Add program version of service

INSERT INTO public.game_skills VALUES (public.random_guid(), 305, 'Resurrect', '4e8d25fe-50f7-11e8-a7ca-02100700d6f0', 512, 0, 'Resurrect target with 25% HP and 0 Energy', 'Resurrect target with 25% HP and 0 Energy', '/Textures/Skills/placeholder.png', '/scripts/skills/resurrect.lua', '73156b15-50f4-11e8-a7ca-02100700d6f0', '', '', 1, 5000, 8000, 10, 0, 0, 0, 0);

UPDATE public.versions SET value = value + 1 WHERE name = 'game_skills';

UPDATE public.versions SET value = 12 WHERE name = 'schema';
