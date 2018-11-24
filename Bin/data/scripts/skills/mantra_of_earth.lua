include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")

costEnergy = 5
costAdrenaline = 0
activation = 0
recharge = 20000
overcast = 0

function onStartUse(source, target)
  return true
end

function onEndUse(source, target)
  source:AddEffect(source, 6, 0)
end

function onCancelUse()
end
