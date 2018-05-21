-- PvP is forever on this map
isPersistent = false

function getDuration(self, source, target, baseDuration)
  return (2 ^ 32) - 1
end

function onStart(self, source, target)
  print("Add Effect PvP to " .. target:GetName())
  return true
end

function onEnd(self, source, target)
end

function onRemove(self, source, target)
  print("Remove Effect PvP from " .. target:GetName())
end

function onUpdate(self, source, target, timeElapsed)
end
