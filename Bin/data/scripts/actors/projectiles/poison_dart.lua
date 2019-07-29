include("/scripts/includes/consts.lua")
include("/scripts/includes/damage.lua")

-- Poison Dart makes 33 damage and applies Poison effect
function onInit()
  self:SetSpeed(4.0)
  return true
end

function onHitTarget(creature)
  -- TODO: Make some index for this "skill"
  local actor = creature:AsActor()
  if (actor ~= nil) then
    actor:ApplyDamage(nil, 0, DAMAGETYPE_PIERCING, 33, 0)
    actor:AddEffect(nil, 10002, 8000)
  end
end
