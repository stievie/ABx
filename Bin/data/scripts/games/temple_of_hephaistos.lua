-- Game start up
function onStart()
  local smith = self:AddNpc("/scripts/creatures/npcs/smith.lua")
  if (smith ~= nil) then
    local x = -6.71275
    local z = 15.5906
    local y = self:GetTerrainHeight(x, z)
    smith:SetPosition(x, y, z)
    smith:SetRotation(180)
  end  
  local merchant = self:AddNpc("/scripts/creatures/npcs/merchant.lua")
  if (merchant ~= nil) then
    local x = 4.92965
    local z = 14.2049
    local y = self:GetTerrainHeight(x, z)
    merchant:SetPosition(x, y, z)
    merchant:SetRotation(180)
  end  
end

-- Game stopping
function onStop()
end

function onAddObject(object)
--  print("Object added: " .. object:GetName())
end

function onRemoveObject(object)
--  print("Object removed: " .. object:GetName())
end

function onPlayerJoin(player)
  player:AddEffect(empty, 1000, 0)
--  print("Player joined: " .. player:GetName())
end

function onPlayerLeave(player)
  player:RemoveEffect(1000)
--  print("Player left: " .. player:GetName())
end

-- Game Update
function onUpdate(timeElapsed)
--  print(timeElapsed)
end
