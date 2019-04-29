include("/scripts/includes/consts.lua")

name = "Chest"
level = 20
modelIndex = 13
sex = SEX_UNKNOWN
creatureState = CREATURESTATE_IDLE
prof1Index = 0
prof2Index = 0

function onInit()
  -- Player collides with BB. Make it a bit larget than the default BB.
  self:SetBoundingSize(1.00349, 0.67497, 0.680545)
  self:SetUndestroyable(true)
  return true
end

function onClicked(creature)
  -- TODO: If in range show chest
end

-- self was selected by creature
function onSelected(creature)
end
