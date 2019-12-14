include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")

costEnergy = 0
costAdrenaline = 0
activation = 3000
recharge = TIME_FOREVER    -- Infinite until morale boost
overcast = 0
range = RANGE_CASTING
effect = SkillEffectResurrect
effectTarget = SkillTargetTarget
targetType = SkillTargetTypeAlly

function onStartUse(source, target)
  if (target == nil) then
    return SkillErrorInvalidTarget
  end
  if (source:GetId() == target:GetId()) then
    -- Can not use this skill on self
    return SkillErrorInvalidTarget
  end;
  if (target:IsDead() == false) then
    return SkillErrorInvalidTarget
  end
  if (self:IsInRange(target) == false) then
    return SkillErrorOutOfRange
  end
  source:FaceObject(target)
  return SkillErrorNone
end

function onSuccess(source, target)
  if (target == nil) then
    return SkillErrorInvalidTarget
  end
  if (target:IsDead() == false) then
    return SkillErrorInvalidTarget
  end

  target:Resurrect(100, 25)
  return SkillErrorNone
end
