include("/scripts/includes/chat.lua")
include("/scripts/actors/consts.lua")

name = "Pedestrian"
level = 20
modelIndex = 12    -- Female Pedestrian 1 body model
sex = SEX_FEMALE
creatureState = CREATURESTATE_IDLE
prof1Index = 5
prof2Index = 0     -- None
behaviours = "/scripts/actors/npcs/behaviours.lua"

local startTick;

function onInit()
  startTick = GameTick()
  self:SetSpeed(0.5)
  return true
end

function onUpdate(timeElapsed)  
  if (GameTick() - startTick > 10000 and self:GetState() == CREATURESTATE_IDLE) then
--    local pos = self:GetPosition();
--    print("going " .. pos[1] .. "," .. pos[2] .. "," .. pos[3])
    self:GotoPosition(-48.85, 0.0, -34.67)
    startTick = GameTick()
  end
end

function onArrived()
  self:SetState(CREATURESTATE_EMOTE_SIT)
end

function onClicked(creature)
  if (self:GetState() == CREATURESTATE_IDLE) then
    self:FollowObject(creature)
  end
end

-- self was selected by creature
function onSelected(creature)
  if (self:GetState() ~= CREATURESTATE_IDLE) then
    self:Say(CHAT_CHANNEL_GENERAL, "Not now!")
  end
end

-- creature collides with self
function onCollide(creature)
end
