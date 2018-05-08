-- Morale is forever but can change
baseDuration = (2 ^ 32) - 1
isPersistent = false

currentMorale = 0

function getDuration(self, source, target)
  return baseDuration
end

function onStart(self, source, target)
  return true
end

function onEnd(self, source, target)
end

function onRemove(self, source, target)
end

function onUpdate(self, source, target, timeElapsed)
end
