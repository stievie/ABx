name = "Guild Lord"
level = 20
modelIndex = 5     -- Smith body model
sex = 2            -- Male
creatureState = 1  -- Idle
prof1Index = 1     -- Warrior
prof2Index = 2     -- Ranger

function onInit(self)
  return true
end

function onUpdate(self, timeElapsed)
end

-- self was selected by creature
function onSelected(self, creature)
end

-- creature collides with self
function onCollide(self, creature)
end
