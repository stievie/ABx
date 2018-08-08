-- PvP is forever on this map
isPersistent = false

function getDuration(source, target, baseDuration)
  return (2 ^ 32) - 1
end

function onStart(source, target)
  print("Add Effect PvP to " .. target:GetName())
  return true
end

function onEnd(source, target)
end

function onRemove(source, target)
  print("Remove Effect PvP from " .. target:GetName())
end

function onUpdate(source, target, timeElapsed)
end
