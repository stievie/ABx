include("/scripts/includes/chat.lua")
include("/scripts/includes/consts.lua")
include("/scripts/includes/attributes.lua")
include("/scripts/includes/skill_consts.lua")

name = "Marianna Gani"
level = 20
itemIndex = 10    -- Female Pedestrian 1 body model
sex = SEX_FEMALE
creatureState = CREATURESTATE_IDLE
prof1Index = PROFESSIONINDEX_MONK
prof2Index = PROFESSIONINDEX_NONE
behavior = "PRIEST"

function onInit()
  self:SetSpeed(0.5)
  self:AddEffect(empty, 900000, 0)
  -- Let's make it a heal machine
  local skillBar = self:GetSkillBar()
  -- Add a heal skill
  skillBar:AddSkill(281)
  -- Instant rezz skill
  skillBar:AddSkill(9996)
  skillBar:SetAttributeValue(ATTRIB_HEALING, 12)
  skillBar:SetAttributeValue(ATTRIB_DEVINE_FAVOUR, 12)
  return true
end

function onUpdate(timeElapsed)
end

function onClicked(creature)
end

-- self was selected by creature
function onSelected(creature)
  self:Say(CHAT_CHANNEL_GENERAL, "Not now!")
end

-- creature collides with self
function onCollide(creature)
end

function onArrived()
end

function onEndUseSkill(skill)
  self:Say(CHAT_CHANNEL_GENERAL, "Phew!")
end

function onStartUseSkill(skill)
end

function onDied()
  self:Say(CHAT_CHANNEL_GENERAL, "Aaaaarrrrrrggghhh")
end

function onResurrected()
  self:Say(CHAT_CHANNEL_GENERAL, "Oh, ty")
end
