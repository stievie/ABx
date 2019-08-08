require("data/scripts/ai/shared")

-- Root
--  - PrioritySelector
--      - Idle
function initMariannaGani()
  -- like PRIEST
  local name = "marianna_gani"
  local rootNode = AI.createTree(name):createRoot("PrioritySelector", name)
  stayAlive(rootNode)
  local healNode = healAlly(rootNode)
--    healNode:addNode("PrioritySelector", "saysomething"):addNode("Say{map,Phew! That was close.}", "say_stuff")
--  rezzAlly(rootNode)
  defend(rootNode)
  idleNode = idle(rootNode)
end
