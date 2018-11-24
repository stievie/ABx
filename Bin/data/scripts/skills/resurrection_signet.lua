include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")

costEnergy = 0
costAdrenaline = 0
activation = 3000
recharge = (2 ^ 32) - 1    -- Infinite until morale boost
overcast = 0

function onStartUse(source, target)
  return true
end

function onEndUse(source, target)
end

function onCancelUse()
end
