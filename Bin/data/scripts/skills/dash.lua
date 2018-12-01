include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")

costEnergy = 5
costAdrenaline = 0
activation = 0
recharge = 8000
overcast = 0
effect = SkillEffecSpeed
effectTarget = SkillTargetSelf

function onStartUse(source, target)
  return SkillErrorNone
end

function onEndUse(source, target)
  source:AddEffect(source, 1043)
end

function onCancelUse()
end
