include("scripts/skills/consts.lua")

costEnergy = 0
costAdrenaline = 0
activation = 0
recharge = 0
overcast = 0

function onStartUse(source, target)
  if (source:IsDead())
    return false;
  return source:Die()
end

function onEndUse(source, target)
end

function onCancelUse()
end
