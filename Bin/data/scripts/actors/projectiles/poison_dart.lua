include("/scripts/includes/chat.lua")
include("/scripts/includes/consts.lua")

name = "Poison Dart"
itemIndex = 5001

function onInit()
  self:SetSpeed(4.0)
  return true
end

function onStart(creature)
  self:SetState(CREATURESTATE_MOVING)
end

function onHitTarget(creature)
  creature:ApplyDamage(nil, self:Index(), DAMAGETYPE_PIERCING, 33, 0)
  creature:AddEffect(nil, 10002, 8000)
end
