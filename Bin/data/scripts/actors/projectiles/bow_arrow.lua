include("/scripts/includes/chat.lua")
include("/scripts/includes/consts.lua")

name = "Arrow"
level = 20
modelIndex = 5000
sex = SEX_UNKNOWN
creatureState = CREATURESTATE_MOVING
prof1Index = 0
prof2Index = 0

function onInit()
  return true
end

function onUpdate(timeElapsed)

end

-- creature: GameObject collides with self
function onCollide(creature)
end

function onStart(creature)
end

function onHitTarget(creature)
end

