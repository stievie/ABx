include("/scripts/includes/chat.lua")
include("/scripts/includes/consts.lua")

name = "Arrow"
itemIndex = 5000

function onInit()
  self:SetSpeed(4.0)
  return true
end

function onStart(creature)
  self:SetState(CREATURESTATE_MOVING)
end

function onHitTarget(creature)
  local source = self:GetSource()
  local weapon = source:GetWeapon()
  -- TODO: 
end

