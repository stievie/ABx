include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")
include("/scripts/includes/damage.lua")
include("/scripts/includes/attributes.lua")

-- Spell
costEnergy = 15
costAdrenaline = 0
activation = 1000
recharge = 2000
overcast = 0
-- HP cost
hp = 0
range = RANGE_CASTING
damageType = DAMAGETYPE_FIRE
effect = SkillEffectDamage
effectTarget = SkillTargetTarget | SkillTargetAoe

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
  if (target:IsDead()) then
    return SkillErrorInvalidTarget
  end

  local attribVal = source:GetAttributeValue(ATTRIB_FIRE)
  local damage = math.floor((attribVal * 6) + 10)
  local burning = (math.floor(attribVal / 2) + 1) * 1000

  local actors = target:GetAlliesInRange(RANGE_ADJECENT)
  for i, actor in ipairs(actors) do
    if (actor:HasEffect(10004)) then
      actor:Damage(source, self:Index(), damageType, damage)
    else
      actor:AddEffect(source, 10004, burning)
    end
  end
  return SkillErrorNone
end
