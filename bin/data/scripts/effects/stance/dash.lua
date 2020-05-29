isPersistent = false

local boost = 0.5

function getDuration(source, target)
  return 3000
end

function getSkillCost(skill, activation, energy, adrenaline, overcast, hp)
  return activation, energy, adrenaline, overcast, hp
end

function onStart(source, target)
  target:AddSpeed(boost)
  return true
end

function onEnd(source, target)
  target:AddSpeed(-boost)
end

function onRemove(source, target)
  target:AddSpeed(-boost)
end
