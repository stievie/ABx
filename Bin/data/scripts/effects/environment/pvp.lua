-- PvP is forever on this map
isPersistent = false

function getDuration(source, target)
  return (2 ^ 32) - 1
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
