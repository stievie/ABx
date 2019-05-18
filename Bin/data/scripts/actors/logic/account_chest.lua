include("/scripts/includes/consts.lua")

name = "Chest"
level = 20
modelIndex = 13
sex = SEX_UNKNOWN
creatureState = CREATURESTATE_CHEST_CLOSED
prof1Index = 0
prof2Index = 0

function onInit()
  -- Player collides with BB. Make it a bit larget than the default BB.
  self:SetBoundingSize(1.00349, 0.67497, 0.680545)
  self:SetUndestroyable(true)
  return true
end

function onClicked(creature)
  if (self:IsInRange(RANGE_TOUCH, creature)) then
    if (self:GetState() == CREATURESTATE_CHEST_OPEN) then
      self:SetState(CREATURESTATE_CHEST_CLOSED)
    else 
      self:SetState(CREATURESTATE_CHEST_OPEN)
    -- TODO: Trigger chest dialog
    end
  end
end
