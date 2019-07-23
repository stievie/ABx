include("/scripts/includes/chat.lua")
include("/scripts/includes/consts.lua")

name = "Dimitris Sallas (Smith)"
level = 20
itemIndex = 5     -- Smith body model
sex = SEX_MALE     -- Male
creatureState = CREATURESTATE_IDLE
prof1Index = 1     -- Warrior
prof2Index = 0     -- None
behavior = "SMITH"

function onInit()
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
