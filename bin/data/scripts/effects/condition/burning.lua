isPersistent = false

function getDuration(source, target)
  return 0
end

function onStart(source, target)
  target:SetHealthRegen(-7)
  return true
end

function onEnd(source, target)
  target:SetHealthRegen(7)
end

-- Effect was removed before ended
function onRemove(source, target)
  target:SetHealthRegen(7)
end
