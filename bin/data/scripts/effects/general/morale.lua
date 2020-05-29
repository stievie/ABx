include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")

-- Morale is forever but can change
isPersistent = false

function getDuration(source, target)
  return TIME_FOREVER
end

function onStart(source, target)
  return true
end

function onEnd(source, target)
end

function onRemove(source, target)
end

function getResources(maxHealth, maxEnergy)
  local target = self:GetTarget()
  if (target == nil) then
    return maxHealth, maxEnergy
  end

  local morale = target:GetMorale()
  local newHealth = maxHealth + math.floor((maxHealth / 100) * morale)
  local newEnergy = maxEnergy + math.floor((maxEnergy / 100) * morale)
  return newHealth, newEnergy
end
