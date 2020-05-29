-- Included by weapons to get damage value
function getDamage(baseMin, baseMax, critical)
  if (critical == true) then
    return math.floor(baseMax)
  end
  return math.floor(((baseMax - baseMin) * Random()) + baseMin)
end
