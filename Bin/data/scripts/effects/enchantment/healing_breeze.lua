include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")

isPersistent = false

local regen = 0

function getDuration(source, target)
  return 15000
end

function onStart(source, target)
  local attribVal = source:GetAttributeValue(ATTRIB_HEALING)
  regen = math.floor((attribVal / 4) + 4)
  target:SetHealthRegen(regen)
  return true
end

function onEnd(source, target)
  target:SetHealthRegen(-regen)
end

-- Effect was removed before ended, so onEnd is not called
function onRemove(source, target)
  target:SetHealthRegen(-regen)
end
