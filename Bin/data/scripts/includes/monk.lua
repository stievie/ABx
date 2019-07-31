-- Monk specific functions

function getDevineFavorHealBonus(source)
  local attr = source:GetAttributeValue(ATTRIB_DEVINE_FAVOUR)
  return attr * 3.2
end
