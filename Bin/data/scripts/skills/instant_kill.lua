include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")

costEnergy = 0
costAdrenaline = 0
activation = 0
recharge = 0
overcast = 0

function onStartUse(source, target)
  if (target:IsDead()) then
    return false
  end
  print("Using Instant Kill on " .. target:GetName())
  return target:Die()
end

function onEndUse(source, target)
end

function onCancelUse()
end
