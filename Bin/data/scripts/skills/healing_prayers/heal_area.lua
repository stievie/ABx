include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")
include("/scripts/includes/damage.lua")
include("/scripts/includes/attributes.lua")
include("/scripts/includes/monk.lua")

-- Touch skill
costEnergy = 10
costAdrenaline = 0
activation = 1000
recharge = 5000
overcast = 0
-- HP cost
hp = 0
range = RANGE_ADJECENT
effect = SkillEffectHeal
effectTarget = SkillTargetTarget | SkillTargetSelf | SkillTargetAoe
targetType = SkillTargetTypeNone

function onStartUse(source, target)
  return SkillErrorNone
end

function onSuccess(source, target)
  local attribVal = source:GetAttributeValue(ATTRIB_HEALING)
  local hp = math.floor(30 + (attribVal * 10))
  local bonus = math.floor(getDevineFavorHealBonus(source))

  local actors = source:GetActorsInRange(range)
  for i, actor in ipairs(actors) do
    actor:Healing(source, self:Index(), hp)
    actor:Healing(source, self:Index(), bonus)
  end
  return SkillErrorNone
end
