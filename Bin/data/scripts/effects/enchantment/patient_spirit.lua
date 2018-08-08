-- 2 Sec
isPersistent = false

function getDuration(source, target, baseDuration)
  return baseDuration
end

function onStart(source, target)
  return true
end

function onEnd(source, target)
  target:Health = target:Health + 120
end

-- Effect was removed before ended
function onRemove(source, target)
end

function onUpdate(source, target, timeElapsed)
end
