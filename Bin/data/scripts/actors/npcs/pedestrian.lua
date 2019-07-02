include("/scripts/includes/chat.lua")
include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")

name = "Marianna Gani"
level = 20
modelIndex = 10    -- Female Pedestrian 1 body model
sex = SEX_FEMALE
creatureState = CREATURESTATE_IDLE
prof1Index = 3     -- Monk
prof2Index = 0     -- None
behavior = "PRIEST"

function onInit()
  self:SetSpeed(0.5)
  self:AddEffect(empty, 900000, 0)
  -- Let's make it a rezz machine :D
  local skillBar = self:GetSkillBar()
  -- Add a heal skill
  skillBar:AddSkill(281)
  -- Instant rezz skill
  skillBar:AddSkill(9996)
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
