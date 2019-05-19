include("/scripts/includes/consts.lua")
include("/scripts/includes/dialogs.lua")

name = "Chest"
level = 20
modelIndex = 13
sex = SEX_UNKNOWN
creatureState = CREATURESTATE_CHEST_CLOSED
prof1Index = 0
prof2Index = 0

function onInit()
  self:SetBoundingSize(1.00349, 0.67497, 0.680545)
  self:SetUndestroyable(true)
  return true
end

function onClicked(creature)
  if (self:IsInRange(RANGE_ADJECENT, creature)) then
    if (self:GetState() == CREATURESTATE_CHEST_OPEN) then
      self:SetState(CREATURESTATE_CHEST_CLOSED)
    else 
      self:SetState(CREATURESTATE_CHEST_OPEN)
    end
    local player = creature:AsPlayer()
    if (player ~= nil) then
      player:TriggerDialog(DIALOG_ACCOUNTCHEST)
    end
  end
end
