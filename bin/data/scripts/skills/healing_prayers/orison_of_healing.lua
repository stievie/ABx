include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")
include("/scripts/includes/damage.lua")
include("/scripts/includes/attributes.lua")
include("/scripts/includes/monk.lua")

-- Touch skill
costEnergy = 5
costAdrenaline = 0
activation = 1000
recharge = 2000
overcast = 0
-- HP cost
hp = 0
range = RANGE_CASTING
effect = SkillEffectHeal
effectTarget = SkillTargetTarget | SkillTargetSelf
targetType = SkillTargetTypeAllyAndSelf

function onStartUse(source, target)
  if (target == nil) then
    -- This skill needs a target
    return SkillErrorInvalidTarget
  end
  if (self:IsInRange(target) == false) then
    -- The target must be in range
    return SkillErrorOutOfRange
  end
  if (source:IsEnemy(target) == true) then
    -- Targets only allies
    return SkillErrorInvalidTarget
  end
  if (target:IsDead()) then
    -- Can not heal what's dead :(
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
  local attribVal = source:GetAttributeRank(ATTRIB_HEALING)
  local hp = math.floor(20 + (attribVal * 3.2))
  target:Healing(source, self:Index(), hp)
  local bonus = math.floor(getDevineFavorHealBonus(source))
  if (bonus ~= 0) then
    target:Healing(source, self:Index(), bonus)
  end
  return SkillErrorNone
end
