include("/scripts/includes/damage.lua")
-- Include drop stats etc.
include("/scripts/includes/hammer_defaults.lua")
include("/scripts/includes/drop_stats.lua")

function getDamage(baseMin, baseMax, critical)
  if (critical == true) then
    return baseMax
  end
  return ((baseMax - baseMin) * math.random()) + baseMin
end
