-- Get drop statistics for damage

function getRandomStat(maxVal, level, minVal)
  if (level == 20 and Random() >= 0.5) then
    -- For level 20 the chance to get max stats is 50%
    return maxVal
  end
  local p = level / 20
  local val = maxVal * p
  local res = val - (2 * Random())
  if (res < minVal) then
    res = minVal
  end
  print(p, val, res)
  return res
end

-- Range is damage / 2 .. damage
function getDamageStats(level)
  local diff = dropStats["MaxDamage"] - dropStats["MinDamage"]
  local maxRes = getRandomStat(dropStats["MaxDamage"], level, 3)
  local p = level / 20
  local  minRes = maxRes - (diff * p)
  if (minRes < 1) then
    minRes = 1
  end
  print(minRes, maxRes)
  return minRes, maxRes
end

function getEnergyStats(level)
  local res = getRandomStat(dropStats["Energy"], level, 1)
  print(res)
  return res
end

function getArmorStats(level)
  local res = getRandomStat(dropStats["Armor"], level, 6)
  print(res)
  return res
end

function getHealthStats(level)
  local res = getRandomStat(dropStats["Health"], level)
  print(res)
  return res
end
