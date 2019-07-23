function onStart()
  local priest = self:AddNpc("/scripts/actors/npcs/priest.lua")
  -- To make them allies set the same group ID
  local groupId = NewGroupId()
  if (priest ~= nil) then
    local x = -6.71275
    local z = 15.5906
    local y = self:GetTerrainHeight(x, z)
    priest:SetPosition({x, y, z})
    priest:SetRotation(180)
    priest:SetGroupId(groupId)
  end
  local guildLord = self:AddNpc("/scripts/actors/npcs/guild_lord.lua")
  if (guildLord ~= nil) then
    local x = -6.71275
    local z = 17.5906
    local y = self:GetTerrainHeight(x, z)
    guildLord:SetPosition({x, y, z})
    guildLord:SetRotation(180)
    guildLord:SetGroupId(groupId)
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
