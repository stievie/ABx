include("/scripts/includes/chat.lua")
include("/scripts/includes/consts.lua")
include("/scripts/includes/attributes.lua")

name = "Dimitris Sallas (Smith)"
level = 20
itemIndex = 5     -- Smith body model
sex = SEX_MALE     -- Male
creatureState = CREATURESTATE_IDLE
prof1Index = PROFESSIONINDEX_WARRIOR     -- Warrior
prof2Index = 0     -- None
behavior = "smith"

function onInit()
  local skillBar = self:GetSkillBar()
  skillBar:SetAttributeValue(ATTRIB_STRENGTH, 12)
  return true
end

function onUpdate(timeElapsed)

end

function onClicked(creature)
  if (creature ~= nil) then
    self:FaceObject(creature)
  end
end

-- self was selected by creature
function onSelected(creature)
  self:Say(CHAT_CHANNEL_GENERAL, "Hello " .. creature:GetName())
end

-- creature collides with self
function onCollide(creature)
end
