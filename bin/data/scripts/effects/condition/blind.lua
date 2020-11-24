include("/scripts/includes/consts.lua")

function onAttacked(source, target, damageType, damage)
  source:SetAttackError(ATTACK_ERROR_TARGET_MISSED)
  return false
end
