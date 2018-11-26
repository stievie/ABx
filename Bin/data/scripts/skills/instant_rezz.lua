include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")

costEnergy = 0
costAdrenaline = 0
activation = 0
recharge = 0
overcast = 0
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
  if (self:IsInRange(target) == false) then
    return SkillErrorOutOfRange
  end
  if (target:IsDead() == false) then
    return SkillErrorInvalidTarget
  end
  return SkillErrorNone
end

function onEndUse(source, target)
  --print("Using Instant Rezz on " .. target:GetName())
  target:Resurrect(100, 100)
end

function onCancelUse()
end
