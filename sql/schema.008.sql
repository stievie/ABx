-- Fix: Wrong proofession

UPDATE public.game_skills SET profession_uuid = '59f4493d-50f4-11e8-a7ca-02100700d6f0' WHERE uuid = 'bc4ff444-50f9-11e8-a7ca-02100700d6f0';

UPDATE public.versions SET value = value + 1 WHERE name = 'game_skills';

UPDATE public.versions SET value = 8 WHERE name = 'schema';
