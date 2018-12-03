include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")

costEnergy = -5
costAdrenaline = 0
activation = 250
recharge = 4000
overcast = 0

function onStartUse(source, target)
  return true
end

function onSuccess(source, target)
  target:AddEffect("Patient Spirit", duration)
end

function getDuration(source, target)
  -- TODO: Different factors influences the duration of enchantments.
  return 2000
end
