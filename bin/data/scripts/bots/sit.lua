include("/scripts/includes/chat.lua")
include("/scripts/includes/consts.lua")

function onInit()
end

function onUpdate(timeElapsed)
  if (self:GetState() ~= CREATURESTATE_EMOTE_SIT) then
    self:Command(14, "")
  end
end
