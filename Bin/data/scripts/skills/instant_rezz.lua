include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")

costEnergy = 10
costAdrenaline = 0
activation = 3000
recharge = 10000
overcast = 0
hp = 50
range = RANGE_ADJECENT
effect = SkillEffectResurrect
effectTarget = SkillTargetTarget

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
  if (target:IsDead() == false) then
    return SkillErrorInvalidTarget
  end
    
  target:Resurrect(100, 100)
  return SkillErrorNone
end

function onCancelled(source, target)
end

function onInterrupted(source, target)
end