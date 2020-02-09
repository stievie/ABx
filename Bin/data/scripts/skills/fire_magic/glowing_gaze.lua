include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")
include("/scripts/includes/damage.lua")
include("/scripts/includes/attributes.lua")

-- Spell
costEnergy = 5
costAdrenaline = 0
activation = 1000
recharge = 8000
overcast = 10
-- HP cost
hp = 0
range = RANGE_CASTING
damageType = DAMAGETYPE_FIRE
effect = SkillEffectDamage
effectTarget = SkillTargetTarget
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

  local fireAttrib = source:GetAttributeRank(ATTRIB_FIRE)
  local damage = math.floor((fireAttrib * 6) + 10)

  target:Damage(source, self:Index(), damageType, damage)
  if (target:HasEffect(10004)) then
    local esAttrib = source:GetAttributeRank(ATTRIB_ENERGY_STORAGE)
    local energy = 5 + math.floor(1 + (esAttrib / 2))
    source:AddEnergy(energy)
  end
  return SkillErrorNone
end
