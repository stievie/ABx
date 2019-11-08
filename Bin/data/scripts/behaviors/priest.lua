include("/scripts/behaviors/shared.lua")

function init(root)
  local prio = self:CreateNode("Priority")
  prio:AddNode(stayAlive())
  prio:AddNode(avoidSelfDamage())
  prio:AddNode(healAlly())
  prio:AddNode(rezzAlly())
  prio:AddNode(defend())
  prio:AddNode(idle(1000))

  root:AddNode(prio)
end
