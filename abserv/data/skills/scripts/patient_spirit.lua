name = "Patient Spirit"
energy = -5
adrenaline = 0
activation = 250
recharge = 4000
overcast = 0
duration = 2000
skilltype = 10 | (1 << 8)  -- SkillTypeEnchantment

function onStartUse(player, target)
  return true
end

function onEndUse(player, target)
  target:AddEffect("Patient Spirit", duration)
end

function onCancelUse()
end
