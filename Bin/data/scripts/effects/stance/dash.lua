isPersistent = false

function getDuration(source, target)
  return 3000
end

function onStart(source, target)
  target:AddSpeed(0.5)
  return true
end

function onEnd(source, target)
  target:AddSpeed(-0.5)
end

function onRemove(source, target)
  target:AddSpeed(-0.5)
end

function onUpdate(source, target, timeElapsed)
end
