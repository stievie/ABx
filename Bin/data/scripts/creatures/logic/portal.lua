name = "Portal"
level = 20
modelIndex = 11
sex = 0
creatureState = 1
prof1Index = 0
prof2Index = 0

function onInit()
  -- Player collides with BB
  self:SetBoundingBox(-1, -1, -1, 1, 1, 1)
  return true
end

function onUpdate(timeElapsed)

end

function onClicked(creature)
end

-- self was selected by creature
function onSelected(creature)
end

-- creature collides with self
function onCollide(creature)
end

function onTrigger(creature)
  print(creature:GetName() .. " triggered")
  local player = creature:AsPlayer()
  if (player ~= nil) then
--    player:ChangeGame(self:GetVarString("destination"))
  end
end
