-- Update skills descritions to use templates

UPDATE public.game_skills SET description = 'Touch Skill. Deal ${10 + (smiting * 3)} holy damage. Deal ${10 + (smiting * 3)} additional damage if target is knocked down.' WHERE uuid = '3713d062-a8e7-4dea-9d9c-90ab9697f8cb';
UPDATE public.game_skills SET short_description = 'Touch Skill. Deal 10..55 holy damage. Deal 10..55 additional damage if target is knocked down.' WHERE uuid = '3713d062-a8e7-4dea-9d9c-90ab9697f8cb';
UPDATE public.game_skills SET description = 'Spell. Target loses 3 energy, you are healed for ${(inspiration * 3) + 20} health by lost energy point.' WHERE uuid = 'eecf3e66-d7fb-499c-872f-2824e34fdae1';
UPDATE public.game_skills SET short_description = 'Spell. Target loses 3 energy, you are healed for 20..65 health by lost energy point.' WHERE uuid = 'eecf3e66-d7fb-499c-872f-2824e34fdae1';
UPDATE public.game_skills SET description = 'Spell. Deal ${10 + (smiting * 3)} holy damage. If target is attacking it takes ${10 + (smiting * 1.5)} additional damage.' WHERE uuid = '97b7b756-19e3-40d7-8e25-3de18f5a07f1';
UPDATE public.game_skills SET description = 'Spell. Heal party for ${30 + (healing * 3)} Health.' WHERE uuid = 'ee6923e1-ef49-4355-a636-1512b51574e1';
UPDATE public.game_skills SET description = 'Spell. Heals for ${20 + (healing * 3.2)}.' WHERE uuid = 'e4fdaa0b-4ecf-49c6-a3f5-ca6119117341';
UPDATE public.game_skills SET description = 'Signet. You get ${82 + (tactics * 6)} Health. You have -40 armor while using this skill.' WHERE uuid = 'bc4ff444-50f9-11e8-a7ca-02100700d6f0';
UPDATE public.game_skills SET description = 'Spell. Heal all adjacent creatures for ${30 + (healing * 10)} points.' WHERE uuid = '1cee4a88-a386-414e-b2eb-dcc2390dcd29';
UPDATE public.game_skills SET description = 'Signet. Interrupt targets action. You gain ${inspiration} energy if it was a spell.' WHERE uuid = 'b1db8982-6914-4150-aa3c-e293f88151a5';
UPDATE public.game_skills SET description = 'Elite Spell. Target foe loses ${domination * 9} Energy. For each lost energy point, nearby foes take 9 damage.' WHERE uuid = 'edf646da-b914-4be1-9b34-58ede0c5c3d4';
UPDATE public.game_skills SET description = 'Spell. Target loses ${domination * 9} Energy and takes 9 damage for each lost energy point.' WHERE uuid = '5595ea05-99fd-44e4-bd15-fb890a21eee1';
UPDATE public.game_skills SET description = 'Enchantment Spell. Target gets +${(healing / 4) + 4} health regeneration (15 seconds).' WHERE uuid = 'd53e8cfa-861e-4986-8631-b72c127cf276';
UPDATE public.game_skills SET description = 'Spell. Target gets ${(fire * 4) + 20} fire damage and burns for ${(fire / 4) + 1} seconds.' WHERE uuid = '6dc7c172-4101-400e-a710-c7439766f128';
UPDATE public.game_skills SET description = 'Enchantment Spell. After 2 seconds target is healed for ${30 + (healing * 6)}.' WHERE uuid = '19ee1bf9-0462-fd18-da74-8e2f6587436f';
UPDATE public.game_skills SET short_description = 'Enchantment Spell. After 2 seconds target is healed for 30..120.' WHERE uuid = '19ee1bf9-0462-fd18-da74-8e2f6587436f';
UPDATE public.game_skills SET description = 'Spell. Deal ${7 * fire} damage and knock down every 3 seconds (9 seconds) to adjacent foes at targets initial location.' WHERE uuid = '688ca399-8f67-4cd3-a6b4-af4c10939d4d';
UPDATE public.game_skills SET description = 'Elite Spell. Burning foes get ${(fire * 6) + 10} fire damage. Foes not burning burn for ${(fire / 2) + 1} seconds.' WHERE uuid = 'e7062cfe-daf2-4b50-8f07-d1b20c032355';
UPDATE public.game_skills SET description = 'Spell. Deal ${(fire * 6) + 10} fire damage. If the target is burning, you get 5 energy plus 1 Energy for every 2 ranks of Energy Storage.' WHERE uuid = 'fab4bcad-5fe1-4616-bf29-b9148740c869';

UPDATE public.versions SET value = value + 1 WHERE name = 'game_skills';

UPDATE public.versions SET value = 7 WHERE name = 'schema';
