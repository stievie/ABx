include("/scripts/includes/consts.lua")

name = ""
level = 20
itemIndex = 11
creatureState = CREATURESTATE_IDLE

function onInit()
  -- Player collides with BB. Make it a bit larget than the default BB.
  self:SetBoundingSize({1.0, 2.0, 2.0})
  self:SetUndestroyable(true)
  -- Will call onTrigger() when it collides
  self:SetTrigger(true)
  return true
end

function onTrigger(creature)
  local player = creature:AsPlayer()
  if (player ~= nil) then
    local party = player:GetParty()
    if (party ~= nil) then
      party:ChangeInstance(self:GetVarString("destination"))
    end
  end
end
