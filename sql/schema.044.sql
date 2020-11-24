-- Add blind condition

INSERT INTO public.game_effects VALUES ('3106c200-8cdd-4bb2-899d-f70daa683d67', 10005, 'Blind', 1, 'scripts/effects/condition/blind.lua', 'Textures/Skills/placeholder.png', '', '');
UPDATE public.versions SET value = value + 1 WHERE name = 'game_effects';

UPDATE public.versions SET value = 44 WHERE name = 'schema';
