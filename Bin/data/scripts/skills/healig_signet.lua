include("scripts/skills/consts.lua")

costEnergy = 0
costAdrenaline = 0
activation = 2000
recharge = 4000
overcast = 0

function onStartUse(source, target)
  return true
end

function onEndUse(source, target)
end

function onCancelUse()
end
