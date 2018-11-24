include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")

costEnergy = 0
costAdrenaline = 0
activation = 0
recharge = 0
overcast = 0
range = RANGE_MAP

function onStartUse(source, target)
  if (target == nil) then
    return false
  end
  if (source:GetId() == target:GetId()) then
    -- Can not use this skill on self
    return false
  end;
  if (self:IsInRange(target) == false) then
    -- This skill needs a target and it must be in range
    return false
  end  
  if (target:IsDead()) then
    -- Can not kill what's already dead :(
    return false
  end
  print("Using Instant Kill on " .. target:GetName())
  return target:Die()
end

function onEndUse(source, target)
end

function onCancelUse()
end
