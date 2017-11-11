name = "Patient Spirit"
duration = 2000

function onStart(source, target)
  return true
end

function onEnd(source, target)
  target:Health = target:Health + 120
end

-- Effect was removed before ended
function onRemoved(source, target)
end
