include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")
include("/scripts/includes/attributes.lua")
include("/scripts/includes/monk.lua")

isPersistent = false

function getDuration(source, target)
  return 2000
end

function onStart(source, target)
  return true
end

function onEnd(source, target)
  -- No effect when it was removed before ended
  if (target == nil) then
    return
  end
  if (target:IsDead()) then
    return
  end
  local attribVal = source:GetAttributeRank(ATTRIB_HEALING)
  local hp = math.floor(30 + (attribVal * 6))
  target:Healing(source, self:Index(), hp)
  local bonus = math.floor(getDevineFavorHealBonus(source))
  if (bonus ~= 0) then
    target:Healing(source, self:Index(), bonus)
  end
end
