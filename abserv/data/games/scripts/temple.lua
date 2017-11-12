function onStart(self)
--  print(self:GetName())
  local npc = self:AddNpc("/objects/scripts/npcs/priest.lua")
end

function onStop(self)
end

function onAddObject(self, object)
  print("Object added: " .. object:GetName())
end

function onPlayerJoin(self, player)
end

function onPlayerLeave(self, player)
end

-- Game Update
function onUpdate(self, timeElapsed)
--  print(timeElapsed)
end
