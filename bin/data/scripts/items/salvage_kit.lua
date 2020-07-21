include("/scripts/includes/item_functions.lua")

dropStats = {}
dropStats["Usages"] = 100

materialStats = {}
materialStats[1] = { 9999999, 1000 }
materialStats[2] = { 0, 0 }
materialStats[3] = { 0, 0 }
materialStats[4] = { 0, 0 }

function onConsume()
  return false
end
