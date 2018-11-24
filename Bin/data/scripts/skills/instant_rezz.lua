include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")

costEnergy = 0
costAdrenaline = 0
activation = 0
recharge = 0
overcast = 0
range = RANGE_TOUCH

function onStartUse(source, target)
  if (target == nil) then
    return false
  end
  if (source:GetId() == target:GetId()) then
    -- Can not use this skill on self
    return false
  end;
  if (self:IsInRange(target) == false) then
    return false
  end
  if (target:IsDead() == false) then
    return false
  end
  return true
end

function onEndUse(source, target)
  print("Using Instant Rezz on " .. target:GetName())
  return target:Resurrect(100, 100)
end

function onCancelUse()
end
