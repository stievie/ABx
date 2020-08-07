include("/scripts/includes/consts.lua")
include("/scripts/includes/dialogs.lua")

name = "Chest"
level = 20
itemIndex = 13
sex = SEX_UNKNOWN
creatureState = CREATURESTATE_CHEST_CLOSED
prof1Index = 0
prof2Index = 0
interactionRange = RANGE_ADJECENT

function onInit()
  self:SetBoundingSize({1.00349, 0.67497, 0.680545})
  self:SetUndestroyable(true)
  return true
end

local function openChest(creature)
  if (self:IsInRange(interactionRange, creature)) then
    if (self:GetState() == CREATURESTATE_CHEST_OPEN) then
      self:SetState(CREATURESTATE_CHEST_CLOSED)
    else
      self:SetState(CREATURESTATE_CHEST_OPEN)
    end
    local player = creature:AsPlayer()
    if (player ~= nil) then
      player:TriggerDialog(self:GetId(), DIALOG_ACCOUNTCHEST)
    end
  end
end

function onClicked(creature)
  openChest(creature)
end

-- creature is interacting with self
function onInteract(creature)
  openChest(creature)
end
