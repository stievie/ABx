include("/scripts/includes/chat.lua")
include("/scripts/includes/consts.lua")
include("/scripts/includes/attributes.lua")

name = "Electra Staneli"
level = 20
itemIndex = 14
sex = SEX_FEMALE
creatureState = CREATURESTATE_IDLE
prof1Index = PROFESSIONINDEX_ELEMENTARIST
prof2Index = PROFESSIONINDEX_NONE
behavior = "elementarist"

local startTick;

function onInit()
  startTick = Tick()
  self:SetSpeed(0.5)

  local skillBar = self:GetSkillBar()
  skillBar:AddSkill(192)
  skillBar:AddSkill(884)
  skillBar:AddSkill(1379)
  skillBar:AddSkill(191)
  skillBar:SetAttributeValue(ATTRIB_ENERGY_STORAGE, 12)
  skillBar:SetAttributeValue(ATTRIB_FIRE, 12)

  return true
end

function onUpdate(timeElapsed)
  if (Tick() - startTick > 10000 and self:GetState() == CREATURESTATE_IDLE) then
    startTick = Tick()
  end
end

function onArrived()
--  self:SetState(CREATURESTATE_EMOTE_SIT)
end

function onClicked(creature)
  if (self:GetState() == CREATURESTATE_IDLE) then
--    self:FollowObject(creature)
  end
end

-- self was selected by creature
function onSelected(creature)
  if (self:GetState() ~= CREATURESTATE_IDLE) then
--    self:Say(CHAT_CHANNEL_GENERAL, "Not now!")
  end
end

-- creature collides with self
function onCollide(creature)
end

function onDied()
end

function onResurrected()
end
