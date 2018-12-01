isPersistent = false

function getDuration(source, target)
  local attrib = source:GetAttributeValue(3)
  return 30 + (4 * attrib)
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
