include("/scripts/includes/consts.lua")

name = ""
level = 20
itemIndex = 11
sex = SEX_UNKNOWN
creatureState = CREATURESTATE_IDLE
prof1Index = 0
prof2Index = 0

function onInit()
  self:SetBoundingBox({-1, -1, -1}, {1, 1, 1})
  self:SetUndestroyable(true)
  -- Will call onTrigger() when it collides
  self:SetTrigger(true)
  -- The call back function
  self:SetVarString("callback", "onTrigger")
  return true
end

function onTrigger(creature)
  -- Call onTrigger in game script
  self:CallGameEvent(self:GetVarString("callback"), creature)
end

-- other: GamneObject
function onLeftArea(other)
end
