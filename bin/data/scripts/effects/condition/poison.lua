-- 2 Sec
isPersistent = false

function getDuration(source, target)
  return 0
end

function onStart(source, target)
  target:SetHealthRegen(-4)
  return true
end

function onEnd(source, target)
  target:SetHealthRegen(4)
end

-- Effect was removed before ended
function onRemove(source, target)
  target:SetHealthRegen(4)
end
