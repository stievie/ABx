costEnergy = 0
costAdrenaline = 0
activation = 3000
recharge = (2 ^ 32) - 1    -- Infinite until morale boost
overcast = 0

function onStartUse(player, target)
  return true
end

function onEndUse(player, target)
end

function onCancelUse()
end
