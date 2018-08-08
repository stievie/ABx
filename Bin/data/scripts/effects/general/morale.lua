-- Morale is forever but can change
baseDuration = (2 ^ 32) - 1
isPersistent = false

currentMorale = 0

function getDuration(source, target)
  return baseDuration
end

function onStart(source, target)
  return true
end

function onEnd(source, target)
end

function onRemove(source, target)
end

function onUpdate(source, target, timeElapsed)
end
