include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")
include("/scripts/includes/damage.lua")
include("/scripts/includes/attributes.lua")

-- Spell
costEnergy = 10
costAdrenaline = 0
activation = 2000
recharge = 20000
overcast = 0
-- HP cost
hp = 0
range = RANGE_CASTING
damageType = DAMAGETYPE_CHAOS
effect = SkillEffectDamage
effectTarget = SkillTargetTarget

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
  local attribVal = source:GetAttributeValue(ATTRIB_DOMINATION)
  local energy = attribVal;
  local dEnergy = target:DrainEnergy(energy)
  local damage = dEnergy * 9
  target:Damage(source, self:Index(), damageType, damage)

  return SkillErrorNone
end
