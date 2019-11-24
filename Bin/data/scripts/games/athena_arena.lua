include("/scripts/includes/consts.lua")

local point_blue = {
  -20.0, 0.0, -27.0
}

local point_red = {
  -19.0, 0.0, -44.0
}

local function createTeam(spawn, frnd, foe, rot)
  local priest = self:AddNpc("/scripts/actors/npcs/priest.lua")
  -- To make them allies set the same group ID
  local groupId = NewGroupId()
  if (priest ~= nil) then
    local x = spawn[1] + Random(-1, 1)
    local z = spawn[3] + Random(-1, 1)
    local y = self:GetTerrainHeight(x, z)
    priest:SetPosition({x, y, z})
    priest:SetRotation(rot)
    priest:SetGroupId(groupId)
    priest:AddFriendFoe(frnd, foe)
  end
  local guildLord = self:AddNpc("/scripts/actors/npcs/guild_lord.lua")
  if (guildLord ~= nil) then
    local x = spawn[1] + Random(-1, 1)
    local z = spawn[3] + Random(-1, 1)
    local y = self:GetTerrainHeight(x, z)
    guildLord:SetPosition({x, y, z})
    guildLord:SetRotation(rot)
    guildLord:SetGroupId(groupId)
    guildLord:AddFriendFoe(frnd, foe)
  end
  local ped2 = self:AddNpc("/scripts/actors/npcs/dorothea_samara.lua")
  if (ped2 ~= nil) then
    local x = spawn[1] + Random(-1, 1)
    local z = spawn[3] + Random(-1, 1)
    local y = self:GetTerrainHeight(x, z)
    ped2:SetPosition({x, y, z})
    ped2:SetRotation(rot)
    ped2:SetHomePos({x, y, z})
    guildLord:SetGroupId(groupId)
    ped2:AddFriendFoe(frnd, foe)
  end
end

function onStart()
  createTeam(point_blue, GROUPMASK_1, GROUPMASK_2, -90)
--  createTeam(point_blue, GROUPMASK_1, GROUPMASK_2, -90)
  createTeam(point_red, GROUPMASK_2, GROUPMASK_1, -90)
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
