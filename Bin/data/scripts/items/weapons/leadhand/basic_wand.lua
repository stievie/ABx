include("/scripts/includes/damage.lua")
-- Include drop stats etc.
include("/scripts/includes/wand_defaults.lua")

function getDamage(baseMin, baseMax, critical)
  if (critical == true) then
    return baseMax
  end
  return ((baseMax - baseMin) * math.random()) + baseMin
end
