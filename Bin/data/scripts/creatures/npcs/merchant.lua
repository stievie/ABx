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

-- self was selected by creature
function onSelected(creature)
  print(creature:GetName() .. " selected me, the " .. self:GetName() .. " :D")
end

-- creature collides with self
function onCollide(creature)
end
