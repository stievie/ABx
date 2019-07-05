include("/scripts/includes/chat.lua")
include("/scripts/includes/consts.lua")

name = "Arrow"
level = 20
itemIndex = 5000
sex = SEX_UNKNOWN
creatureState = CREATURESTATE_MOVING
prof1Index = 0
prof2Index = 0

function onInit()
  self:SetSpeed(4.0)
  return true
end

-- creature: GameObject collides with self
function onCollide(creature)
end

function onStart(creature)
end

function onHitTarget(creature)
  local source = self:GetSource()
  local weapon = source:GetWeapon()
  -- TODO: 
end

