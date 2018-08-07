function onStart(self)
  local smith = self:AddNpc("/scripts/creatures/npcs/smith.lua")
  if (smith ~= nil) then
    smith:SetPosition(-6.71275, 25.3445, 12.5906)
    smith:SetRotation(180)
  end  
  local merchant = self:AddNpc("/scripts/creatures/npcs/merchant.lua")
  if (merchant ~= nil) then
    merchant:SetPosition(4.92965, 25.3497, 12.2049)
    merchant:SetRotation(180)
  end  
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
