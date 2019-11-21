include("/scripts/behaviors/shared.lua")

function init(root)
  local prio = node("Priority")
    prio:AddNode(stayAlive())
    prio:AddNode(avoidSelfDamage())
    prio:AddNode(healAlly())
    prio:AddNode(rezzAlly())
    prio:AddNode(defend())
    prio:AddNode(checkEnergy())
    prio:AddNode(goHome())
    prio:AddNode(idle(1000))

  root:AddNode(prio)
end
