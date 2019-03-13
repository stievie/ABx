include("/scripts/includes/damage.lua")
-- Include drop stats etc.
include("/scripts/includes/wand_defaults.lua")

function getDamage(baseMin, baseMax)
  return ((baseMax - baseMin) * math.random()) + baseMin
end
