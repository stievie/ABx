require("data/scripts/ai/shared")

-- Root
--  - PrioritySelector
--      - Idle
function initMariannaGani()
  -- like PRIEST
  local name = "marianna_gani"
  local rootNode = AI.createTree(name):createRoot("PrioritySelector", name)
  stayAlive(rootNode)
  healAlly(rootNode)
--  rezzAlly(rootNode)
  defend(rootNode)
  idle(rootNode)
end
