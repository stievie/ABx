include("/scripts/includes/consts.lua")

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
    priest:AddFriendFoe(GROUPMASK_1, GROUPMASK_2)
  end
  local guildLord = self:AddNpc("/scripts/actors/npcs/guild_lord.lua")
  if (guildLord ~= nil) then
    local x = -6.71275
    local z = 17.5906
    local y = self:GetTerrainHeight(x, z)
    guildLord:SetPosition({x, y, z})
    guildLord:SetRotation(180)
    guildLord:SetGroupId(groupId)
    guildLord:AddFriendFoe(GROUPMASK_1, GROUPMASK_2)
  end
  local ped2 = self:AddNpc("/scripts/actors/npcs/dorothea_samara.lua")
  if (ped2 ~= nil) then
    local x = -4.08
    local z = 18.6
    local y = self:GetTerrainHeight(x, z)
    ped2:SetPosition({x, y, z})
    ped2:SetRotation(180)
    ped2:SetHomePos({x, y, z})
    ped2:AddFriendFoe(GROUPMASK_1, GROUPMASK_2)
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
  player:AddFriendFoe(GROUPMASK_2, GROUPMASK_1)
end

function onPlayerLeave(player)
end

-- Game Update
function onUpdate(timeElapsed)
--  print(timeElapsed)
end
