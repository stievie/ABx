include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")
include("/scripts/includes/damage.lua")
include("/scripts/includes/attributes.lua")
include("/scripts/includes/monk.lua")

-- Touch skill
costEnergy = 15
costAdrenaline = 0
activation = 2000
recharge = 2000
overcast = 0
-- HP cost
hp = 0
range = RANGE_CASTING
effect = SkillEffectHeal
effectTarget = SkillTargetTarget | SkillTargetSelf | SkillTargetParty

function onStartUse(source, target)
  return SkillErrorNone
end

function onSuccess(source, target)
  local attribVal = source:GetAttributeValue(ATTRIB_HEALING)
  local hp = math.floor(30 + (attribVal * 3))
  local bonus = math.floor(getDevineFavorHealBonus(source))

--  target:Healing(source, self:Index(), hp)
--  if (bonus ~= 0) then
--    target:Healing(source, self:Index(), bonus)
--  end
  return SkillErrorNone
end
