include("/scripts/includes/consts.lua")

name = ""
level = 20
modelIndex = 11
sex = SEX_UNKNOWN
creatureState = CREATURESTATE_IDLE
prof1Index = 0
prof2Index = 0

function onInit()
  -- Player collides with BB. Make it a bit larget than the default BB.
  self:SetBoundingBox(-1.5, -1.5, -1, 1.5, 1.5, 1)
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
