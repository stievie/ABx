include("/scripts/includes/chat.lua")
include("/scripts/includes/consts.lua")

name = "Name"
level = 20
modelIndex = 5     -- Smith body model
sex = SEX_MALE     -- Male
creatureState = CREATURESTATE_IDLE
prof1Index = 1     -- Warrior
prof2Index = 0     -- None
behavior = "behaviour"

function onInit()
  return true
end

function onUpdate(timeElapsed)

end

function onClicked(creature)
end

-- self was selected by creature
function onSelected(creature)
end

-- creature collides with self
function onCollide(creature)
end

function onArrived()
end

-- other: Actor
function onTrigger(other)
end

function OnStartUseSkill(skill)
end

function OnEndUseSkill(skill)
end

-- This Actor is attacking the target
-- target: Actor
-- success: bool
function onAttack(target, success)
end

-- This Actor was attacked by source
-- source: Actor
-- type: DamageType
-- damage: int
-- success: bool
function onAttacked(source, _type, damage, success)
end

-- This Actor is going to bee attacked by source. Happens before onAttacked.
-- source: Actor
-- success: bool
function onGettingAttacked(source, success)
end

-- This actor is using a skill on target
-- target: Actor
-- skill: Skill
-- success: bool
function onUseSkill(target, skill, success)
end

-- This Actor is targeted for a skill by source
-- source: Actor
-- skill: Skill
-- success: bool
function onSkillTargeted(source, skill, success)
end
