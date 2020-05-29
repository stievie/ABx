include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")
include("/scripts/includes/damage.lua")
include("/scripts/includes/attributes.lua")

-- Spell
costEnergy = 25
costAdrenaline = 0
activation = 5000
recharge = 60000
overcast = 10
-- HP cost
hp = 0
range = RANGE_CASTING
damageType = DAMAGETYPE_FIRE
effect = SkillEffectDamage
effectTarget = SkillTargetTarget | SkillTargetAoe
targetType = SkillTargetTypeFoe

local position;

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
  position = target:GetPosition()
  return SkillErrorNone
end

function onSuccess(source, target)
  source:AddAOE("/scripts/actors/aoe/elementarist/meteor_shower.lua", self:Index(), position)
  return SkillErrorNone
end
