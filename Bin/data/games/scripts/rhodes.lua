function onStart(self)
--  print(self:GetName())
--  for i = 1, 100 do
--    self:AddNpc("/objects/npcs/scripts/priest.lua")
--    self:AddNpc("/objects/npcs/scripts/guild_lord.lua")
--  end
end

function onStop(self)
end

function onAddObject(self, object)
--  print("Object added: " .. object:GetName())
end

function onRemoveObject(self, object)
--  print("Object added: " .. object:GetName())
end

function onPlayerJoin(self, player)
--  print("Player joined: " .. player:GetName())
end

function onPlayerLeave(self, player)
--  print("Player left: " .. player:GetName())
end

-- Game Update
function onUpdate(self, timeElapsed)
--  print(timeElapsed)
end
