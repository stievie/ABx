include("/scripts/includes/chat.lua")
include("/scripts/includes/consts.lua")

name = "Guild Lord"
level = 20
modelIndex = 5     -- Smith body model
sex = SEX_MALE
creatureState = CREATURESTATE_IDLE
prof1Index = 1     -- Warrior
prof2Index = 2     -- Ranger
behavior = "GUILDLORD"

function onInit()
  local skillBar = self:GetSkillBar()
  -- Add a heal skill
  skillBar:AddSkill(1)
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

function onDied()
  self:DropRandomItem()
end
