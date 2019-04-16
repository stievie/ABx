require("data/scripts/ai/shared")

-- Root
--  - PrioritySelector
--      - Idle
function initPriest()
  local name = "PRIEST"
	local rootNode = AI.createTree(name):createRoot("PrioritySelector", name)
	stayAlive(rootNode)
	healAlly(rootNode)
--	rezzAlly(rootNode)
	defend(rootNode)
	idle(rootNode)
end
