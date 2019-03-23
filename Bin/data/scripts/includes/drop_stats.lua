-- Get drop statistics for damage

-- Range is damage / 2 .. damage
function getDamageStats(level)
  local p = level / 20
  local minD = dropStats["MinDamage"] / p;
  local maxD = dropStats["MaxDamage"] / p;
  local minRes = (((minD / 2) - minD) * math.random()) + (minD / 2)
  local maxRes = (((maxD / 2) - maxD) * math.random()) + (maxD / 2)
  return minRes, maxRes
end

function getEnergyStats(level)
  local p = level / 20
  local energy = dropStats["Energy"] / p;
  local res = (((energy / 2) - energy) * math.random()) + (energy / 2)
  return res
end

function getArmorStats(level)
  local p = level / 20
  local armor = dropStats["Armor"] / p;
  local res = (((armor / 2) - armor) * math.random()) + (armor / 2)
  return res
end
