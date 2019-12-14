include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")
include("/scripts/includes/attributes.lua")

costEnergy = 0
costAdrenaline = 0
activation = 2000
recharge = 4000
overcast = 0
range = RANGE_TOUCH
effect = SkillEffectHeal
effectTarget = SkillTargetSelf
targetType = SkillTargetTypeNone

function onStartUse(source, target)
  return SkillErrorNone
end

function onSuccess(source, target)
  if (source:IsDead()) then
    return SkillErrorInvalidTarget
  end
  local attribVal = source:GetAttributeValue(ATTRIB_TACTICS)
  local hp = 82 + (attribVal * 6)
  source:Healing(source, self:Index(), hp)
  return SkillErrorNone
end
