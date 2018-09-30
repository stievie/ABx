include("/scripts/includes/chat.lua")
include("/scripts/actors/consts.lua")

name = "Pedestrian"
level = 20
modelIndex = 10    -- Female Pedestrian 1 body model
sex = SEX_FEMALE
creatureState = CREATURESTATE_IDLE
prof1Index = 3     -- Monk
prof2Index = 0     -- None

local startTick;

function onInit()
  startTick = GameTick()
  self:SetSpeed(0.5)
  return true
end

function onUpdate(timeElapsed)  
  if (GameTick() - startTick > 10000 and self:GetState() == 1) then
--    print("going")
--    self:GotoPosition(4.92965, 0.0, 14.2049)
  end
end

function onClicked(creature)
  self:FollowObject(creature)
end

-- self was selected by creature
function onSelected(creature)
  self:Say(CHAT_CHANNEL_GENERAL, "Not now!")
end

-- creature collides with self
function onCollide(creature)
end
