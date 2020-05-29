-- 2 Sec
isPersistent = false

local factor = -0.5

function getDuration(source, target)
  return 2000
end

function onStart(source, target)
  target:AddSpeed(factor)
  return true
end

function onEnd(source, target)
  target:AddSpeed(-factor)
end

-- Effect was removed before ended
function onRemove(source, target)
  target:AddSpeed(-factor)
end
