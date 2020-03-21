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
targetType = SkillTargetTypeNone

function onStartUse(source, target)
  return SkillErrorNone
end

function onSuccess(source, target)
  local attribVal = source:GetAttributeRank(ATTRIB_HEALING)
  local hp = math.floor(30 + (attribVal * 3))
  local bonus = math.floor(getDevineFavorHealBonus(source))
  local group = source:GetGroup();
  local members = group:GetMembers()
  
  for i, member in ipairs(members) do
    member:Healing(source, self:Index(), hp)
    member:Healing(source, self:Index(), bonus)
  end
  return SkillErrorNone
end
