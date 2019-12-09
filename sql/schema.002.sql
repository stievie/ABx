
INSERT INTO public.game_effects (idx, name, category, script) VALUES (2061, 'Patient Spirit', 2, '/scripts/effects/enchantment/patient_spirit.lua');
INSERT INTO public.game_skills (idx, name, attribute_uuid, type, description, icon, script, profession_uuid) VALUES (2061, 'Patient Spirit', '9f91d5ba-50f5-11e8-a7ca-02100700d6f0', 512, 'Enchantment Spell. After 2 seconds target is healed for 30..120.', '/Textures/Skills/placeholder.png', '/scripts/skills/healing_prayers/patient_spirit.lua', '73156b15-50f4-11e8-a7ca-02100700d6f0');

UPDATE public.versions SET value = value + 1 WHERE name = 'game_effects';
UPDATE public.versions SET value = value + 1 WHERE name = 'game_skills';

UPDATE public.versions SET value = 2 WHERE name = 'schema';
