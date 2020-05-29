include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")
include("/scripts/includes/damage.lua")
include("/scripts/includes/attributes.lua")

-- Touch skill
costEnergy = 5
costAdrenaline = 0
activation = 1000
recharge = 8000
overcast = 0
-- HP cost
hp = 0
range = RANGE_CASTING
damageType = DAMAGETYPE_UNKNOWN
effect = SkillEffectHeal
-- To whom effect applies
effectTarget = SkillTargetSelf
-- What must be the target
targetType = SkillTargetTypeFoe

function onStartUse(source, target)
  if (target == nil) then
    -- This skill needs a target
    return SkillErrorInvalidTarget
  end
  if (source:GetId() == target:GetId()) then
    -- Can not use this skill on self
    return SkillErrorInvalidTarget
  end
  if (self:IsInRange(target) == false) then
    -- The target must be in range
    return SkillErrorOutOfRange
  end
  if (source:IsEnemy(target) == false) then
    -- Targets only enemies
    return SkillErrorInvalidTarget
  end
  if (target:IsDead()) then
    -- Can not kill what's already dead :(
    return SkillErrorInvalidTarget
  end
  source:FaceObject(target)
  return SkillErrorNone
end

function onSuccess(source, target)
  if (target == nil) then
    return SkillErrorInvalidTarget
  end
  if (target:IsDead()) then
    return SkillErrorInvalidTarget
  end
  local attribVal = source:GetAttributeRank(ATTRIB_INSPIRATION)
  local energy = target:DrainEnergy(3)
  local health = energy * ((attribVal * 3) + 20)
  source:Healing(source, self:Index(), health)
  return SkillErrorNone
end
