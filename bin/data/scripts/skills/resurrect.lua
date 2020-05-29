include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")

costEnergy = 10
costAdrenaline = 0
activation = 5000
recharge = 8000
overcast = 0
range = RANGE_CASTING
effect = SkillEffectResurrect
effectTarget = SkillTargetTarget
targetType = SkillTargetTypeAllyWithoutSelf

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

  target:Resurrect(25, 0)
  return SkillErrorNone
end
