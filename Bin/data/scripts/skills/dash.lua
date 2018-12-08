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

function onSuccess(source, target)
  if (source:IsDead()) then
    return SkillErrorInvalidTarget
  end
  
  source:AddEffect(source, 1043)
  return SkillErrorNone
end
