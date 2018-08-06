function onStart(self)
  self:AddNpc("/scripts/creatures/npcs/smith.lua")
end

function onStop(self)
end

function onAddObject(self, object)
  print("Object added: " .. object:GetName())
end

function onRemoveObject(self, object)
  print("Object removed: " .. object:GetName())
end

function onPlayerJoin(self, player)
  player:AddEffect(empty, 1000, 0)
  print("Player joined: " .. player:GetName())
end

function onPlayerLeave(self, player)
  player:RemoveEffect(1000)
  print("Player left: " .. player:GetName())
end

-- Game Update
function onUpdate(self, timeElapsed)
--  print(timeElapsed)
end
