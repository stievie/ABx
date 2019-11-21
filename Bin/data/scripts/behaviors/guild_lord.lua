include("/scripts/behaviors/shared.lua")

function init(root)
  local prio = node("Priority")
    prio:AddNode(stayAlive())
    prio:AddNode(defend())
    prio:AddNode(damageSkill())
    prio:AddNode(attackAggro())
    prio:AddNode(avoidSelfDamage())
    prio:AddNode(checkEnergy())
    prio:AddNode(goHome())
    prio:AddNode(idle(1000))

  root:AddNode(prio)
end
