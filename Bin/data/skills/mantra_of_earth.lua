costEnergy = 5
costAdrenaline = 0
activation = 0
recharge = 20000
overcast = 0

function onStartUse(player, target)
  return true
end

function onEndUse(player, target)
  player:AddEffect(player, 6, 0)
end

function onCancelUse()
end
