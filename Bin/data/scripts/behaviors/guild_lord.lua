include("/scripts/behaviors/shared.lua")

function init(root)
  local prio = node("Priority")
    prio:AddNode(stayAlive())
    prio:AddNode(avoidSelfDamage())
    prio:AddNode(defend())
    prio:AddNode(damageSkill())
    prio:AddNode(attackAggro())
    prio:AddNode(checkEnergy())
    prio:AddNode(goHome())

  root:AddNode(prio)
end
