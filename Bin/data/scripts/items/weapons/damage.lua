-- Included by weapons to get damage value
function getDamage(baseMin, baseMax, critical)
  if (critical == true) then
    return baseMax
  end
  return ((baseMax - baseMin) * Random()) + baseMin
end
