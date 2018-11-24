include("scripts/skills/consts.lua")

costEnergy = 0
costAdrenaline = 0
activation = 0
recharge = 0
overcast = 0

function onStartUse(source, target)
  if (target:IsDead() == false) then
    return false
  end
  print("Using Instant Rezz on " .. target:GetName())
  return target:Resurrect(100, 100)
end

function onEndUse(source, target)
end

function onCancelUse()
end
