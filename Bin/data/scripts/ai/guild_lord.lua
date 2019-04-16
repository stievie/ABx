require("data/scripts/ai/shared")

-- Root
--  - PrioritySelector
--      - Idle
function initGuildLord()
  local name = "GUILDLORD"
  local rootNode = AI.createTree(name):createRoot("PrioritySelector", name)
  stayAlive(rootNode)
  defend(rootNode)
  attackAggro(rootNode)
  idle(rootNode)
end
