require("skills/_types")

costEnergy = -5
costAdrenaline = 0
activation = 250
recharge = 4000
overcast = 0

function onStartUse(player, target)
  return true
end

function onEndUse(player, target)
  target:AddEffect("Patient Spirit", duration)
end

function onCancelUse()
end

function getDuration(player, target)
  -- TODO: Different factors influences the duration of enchantments.
  return 2000
end
