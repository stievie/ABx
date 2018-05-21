-- 2 Sec
isPersistent = false

function getDuration(self, source, target, baseDuration)
  return baseDuration
end

function onStart(self, source, target)
  return true
end

function onEnd(self, source, target)
  target:Health = target:Health + 120
end

-- Effect was removed before ended
function onRemove(self, source, target)
end

function onUpdate(self, source, target, timeElapsed)
end
