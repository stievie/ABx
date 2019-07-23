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
  local game = target:GetGame()
  local position = target:GetPosition();
  local aoe = game:AddAreaOfEffect("/scripts/actors/aoe/elementarist/meteor_shower.lua",
    source, self:Index(), position)
  return SkillErrorNone
end
