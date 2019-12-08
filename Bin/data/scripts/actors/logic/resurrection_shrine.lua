include("/scripts/includes/consts.lua")

name = ""
level = 20
-- TODO: Make an item for it
itemIndex = 11
creatureState = CREATURESTATE_IDLE

local rezzInterval = 2 * 60 * 1000
local actors = {}
local timePassed = 0

function onInit()
  self:SetUndestroyable(true)
  self:SetTrigger(true)
  timePassed = 0
  return true
end

function onTrigger(creature)
  table.insert(actors, creature:GetId())
end

local function doRezz()
  local pos = self:GetPosition()
  for i, id in ipairs(actors) do
    local object = self:GetGame():GetObject(id)
    if (object ~= nil) then
      local actor = object:AsActor()
      if (actor ~= nil) then
        if (actor:IsDead()) then
          actor:SetPosition(pos)
          actor:Resurrect(100, 100)
        end
      end
    end
  end
end

function onUpdate(timeElapsed)
  timePassed = timePassed + timeElapsed
  if (timePassed >= rezzInterval)
    doRezz()
    timePassed = 0
  end
end
