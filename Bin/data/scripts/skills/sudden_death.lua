include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")

costEnergy = 0
costAdrenaline = 0
activation = 0
recharge = 0
overcast = 0

function onStartUse(source, target)
  if (source:IsDead()) then
    return false
  end
  return true
end

function onEndUse(source, target)
  print("Using Sudden Death")
  return source:Die()
end

function onCancelUse()
end
