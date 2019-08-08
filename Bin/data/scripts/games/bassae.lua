function onStart()
  local ped2 = self:AddNpc("/scripts/actors/npcs/marianna_gani.lua")
  if (ped2 ~= nil) then
    local x = 4.92965
    local z = 5.2049
    local y = self:GetTerrainHeight(x, z)
    ped2:SetPosition({x, y, z})
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
