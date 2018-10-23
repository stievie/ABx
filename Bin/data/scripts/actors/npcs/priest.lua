include("/scripts/includes/chat.lua")
include("/scripts/actors/consts.lua")

name = "Priest"
level = 20
modelIndex = 2
sex = SEX_MALE
creatureState = CREATURESTATE_IDLE
prof1Index = 3     -- Monk
prof2Index = 0     -- None

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
