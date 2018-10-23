function onStart()
  local priest = self:AddNpc("/scripts/actors/npcs/priest.lua")
  if (priest ~= nil) then
    local x = -6.71275
    local z = 15.5906
    local y = self:GetTerrainHeight(x, z)
    priest:SetPosition(x, y, z)
    priest:SetRotation(180)
  end
end

function onStop()
end

function onAddObject(object)
--  print("Object added: " .. object:GetName())
end

function onRemoveObject(object)
--  print("Object added: " .. object:GetName())
end

function onPlayerJoin(player)
--  print("Player joined: " .. player:GetName())
end

function onPlayerLeave(player)
--  print("Player left: " .. player:GetName())
end

-- Game Update
function onUpdate(timeElapsed)
--  print(timeElapsed)
end
