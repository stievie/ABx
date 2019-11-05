include("/scripts/behaviors/shared.lua")

function init(root)
  local prio = self:CreateNode("Priority")
  prio:AddNode(stayAlive())
  prio:AddNode(defend())
  prio:AddNode(attackAggro())
  prio:AddNode(idle(1000))

  root:AddNode(prio)
end
