name = "Temple of Athene"
level = 20
modelIndex = 11
sex = 0
creatureState = 1
prof1Index = 0
prof2Index = 0

function onInit()
  -- Player collides with BB. Make it a bit larget than the default BB.
  self:SetBoundingBox(-1, -1, -1, 1, 1, 1)
  return true
end

function onTrigger(creature)
  local player = creature:AsPlayer()
  if (player ~= nil) then
    player:ChangeGame(self:GetVarString("destination"))
  end
end
