name = "Patient Spirit"
costEnergy = 5
costAdrenaline = 0
activation = 250
recharge = 4000
overcast = 0

function onStartUse(player, target)
  return true
end

function onEndUse(player, target)
  target:AddEffect("Patient Spirit")
end

function onCancelUse()
end
