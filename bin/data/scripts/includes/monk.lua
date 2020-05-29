-- Monk specific functions

function getDevineFavorHealBonus(source)
  local attr = source:GetAttributeRank(ATTRIB_DEVINE_FAVOUR)
  return attr * 3.2
end
