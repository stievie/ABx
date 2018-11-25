include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")

costEnergy = 0
costAdrenaline = 0
activation = 0
recharge = 0
overcast = 0
effect = SkillEffectDamage | SkillTargetSelf

function onStartUse(source, target)
  if (source:IsDead()) then
    -- Redundant check, because you can't use skills when you are dead
    return false
  end
  return true
end

function onEndUse(source, target)
  return source:Die()
end

function onCancelUse()
end
