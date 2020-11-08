include("/scripts/includes/consts.lua")
include("/scripts/includes/create_npcs.lua")

function onStart()
  createPortal(self, 149.139, 22.5063, -31.5364, "Rhodes Arena", "a13b71f8-fe19-4bf5-bba7-c7642c796c0f")
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
