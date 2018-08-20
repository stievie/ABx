name = "Merchant"
level = 20
modelIndex = 9     -- Merchant body model
sex = 2            -- Male
creatureState = 1  -- Idle
prof1Index = 1     -- Warrior
prof2Index = 0     -- None

function onInit()
  return true
end

function onUpdate(timeElapsed)

end

function onClicked(creature)
end

-- self was selected by creature
function onSelected(creature)
--  print(creature:GetName() .. " selected me, the " .. self:GetName() .. " :D")
  -- Testing Raycast
--  local pos = creature:GetPosition();
--  print("Raycast to " .. pos[1] .. "," .. pos[2] .. "," .. pos[3])
--  local objects = self:Raycast(pos[1], pos[2], pos[3]);
--  for i, v in ipairs(objects) do
--    print(i, v, v:GetName()) 
--  end
end

-- creature collides with self
function onCollide(creature)
end
