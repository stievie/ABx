include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")

costEnergy = 5
costAdrenaline = 0
activation = 750
recharge = 3000
overcast = 0

function onStartUse(source, target)
  if (source:GetGame():GetType() < GAMETYPE_PVPCOMBAT)
    return false
  end
  return true
end

function onSuccess(source, target)
end
