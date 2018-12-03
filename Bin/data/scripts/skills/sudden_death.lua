include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")

costEnergy = 0
costAdrenaline = 0
activation = 0
recharge = 0
overcast = 0
effect = SkillEffectDamage
effectTarget = SkillTargetSelf

function onStartUse(source, target)
  if (source:IsDead()) then
    -- Redundant check, because you can't use skills when you are dead
    return SkillErrorInvalidTarget
  end
  return SkillErrorNone
end

function onSuccess(source, target)
  source:Die()
end
