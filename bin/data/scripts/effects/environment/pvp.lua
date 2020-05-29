include("/scripts/includes/consts.lua")

-- PvP is forever on this map
isPersistent = false

function getDuration(source, target)
  return TIME_FOREVER
end

function onStart(source, target)
  return true
end

function onEnd(source, target)
end

function onRemove(source, target)
end
