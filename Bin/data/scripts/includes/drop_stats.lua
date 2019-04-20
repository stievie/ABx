-- Get drop statistics for damage

function getRandomStat(maxVal, level, minVal)
  if (level == 20 and Random() >= 0.3) then
    -- For level 20 the chance to get max stats is 70%
    return maxVal
  end
  local p = level / 20
  local val = maxVal * p
  local res = val - (2 * Random())
  if (res < minVal) then
    res = minVal
  end
  return math.floor(res)
end

-- Range is damage / 2 .. damage
function getDamageStats(level, maxStats)
  if (maxStats) then
    return dropStats["MinDamage"], dropStats["MaxDamage"]
  end

  local maxRes = getRandomStat(dropStats["MaxDamage"], level, 3)
  if (maxRes == dropStats["MaxDamage"]) then
    -- Max -> return max min damage
    return dropStats["MinDamage"], maxRes
  end
  
  local diff = dropStats["MaxDamage"] - dropStats["MinDamage"]
  local p = level / 20
  local  minRes = math.floor(maxRes - (diff * p))
  if (minRes < 1) then
    minRes = 1
  end
  return minRes, maxRes
end

function getEnergyStats(level, maxStats)
  if (maxStats) then
    return dropStats["Energy"]
  end
  return getRandomStat(dropStats["Energy"], level, 1)
end

function getArmorStats(level, maxStats)
  if (maxStats) then
    return dropStats["Armor"]
  end
  return getRandomStat(dropStats["Armor"], level, 6)
end

function getHealthStats(level, maxStats)
  if (maxStats) then
    return dropStats["Health"]
  end
  return getRandomStat(dropStats["Health"], level)
end
