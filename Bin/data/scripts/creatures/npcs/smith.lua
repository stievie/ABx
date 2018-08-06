name = "Smith"
level = 20
modelIndex = 5     -- Smith body model
sex = 2            -- Male
creatureState = 1  -- Idle
prof1Index = 1     -- Warrior
prof2Index = 0     -- None

function onInit(self)
  self:SetPosition(-6.71275, 25.3445, 16.5906)
  self:SetRotation(180)
--  local pos = self:GetPosition()
--  print(pos[1], pos[2], pos[3])
  return true
end

function onUpdate(self, timeElapsed)
end

-- self was selected by creature
function onSelected(self, creature)
  print(creature:GetName() .. " selected me :D")
end
