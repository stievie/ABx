include("/scripts/includes/chat.lua")
include("/scripts/includes/consts.lua")
include("/scripts/includes/attributes.lua")
include("/scripts/includes/skill_consts.lua")

name = "Guild Lord"
level = 20
itemIndex = 5     -- Smith body model
sex = SEX_MALE
creatureState = CREATURESTATE_IDLE
prof1Index = 1     -- Warrior
prof2Index = 2     -- Ranger
behavior = "guild_lord"

function onInit()
  local skillBar = self:GetSkillBar()
  -- Add a heal skill
  skillBar:AddSkill(1)
  skillBar:SetAttributeValue(ATTRIB_TACTICS, 12)
  skillBar:SetAttributeValue(ATTRIB_STRENGTH, 12)
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
