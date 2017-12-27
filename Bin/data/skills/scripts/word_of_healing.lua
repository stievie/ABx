name = "Word of Healing"
costEnergy = 5
costAdrenaline = 0
activation = 750
recharge = 3000
overcast = 0
skilltype = 10  -- SkillTypeSpell

function onStartUse(player, target)
  return true
end

function onEndUse(player, target)
end

function onCancelUse()
end
