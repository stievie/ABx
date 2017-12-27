name = "Patient Spirit"
category = 2 -- Enchantment

function onStart(target)
  return true
end

function onEnd(target)
  target:Health = target:Health + 120
end

-- Effect was removed before ended
function onRemove(target)
end

function onUpdate(timeElapsed)
end
