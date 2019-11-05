function idle(time)
  local idle = self:CreateNode("Idle", time)
  return idle
end

function stayAlive()
  -- Execute the first child that does not fail
  local node = self:CreateNode("Priority")
  local condition = self:CreateCondition("IsSelfHealthLow")
  -- If we have low HP
  node:SetCondition(condition)
  -- 1. try to heal
  node:AddNode(self:CreateNode("HealSelf"))
  -- 2. If that failes flee
--  node:AddNode(self:CreateNode("Flee"))
  return node
end

function defend()
  local node = self:CreateNode("Sequence")
  -- If we are getting attackend AND there is an attacker
  local andCond = self:CreateCondition("And")
  andCond:AddCondition(self:CreateCondition("IsAttacked"))
  local haveAttackers = self:CreateCondition("Filter")
  haveAttackers:SetFilter(self:CreateFilter("SelectAttackers"))
  andCond:AddCondition(haveAttackers)
  node:SetCondition(andCond)
  node:AddNode(self:CreateNode("AttackSelection"))
  return node
end

function healAlly()
  local node = self:CreateNode("Sequence")
  local andCond = self:CreateCondition("And")
  andCond:AddCondition(self:CreateCondition("IsAllyHealthLow"))
  local haveAttackers = self:CreateCondition("Filter")
  haveAttackers:SetFilter(self:CreateFilter("SelectLowHealth"))
  andCond:AddCondition(haveAttackers)
  node:SetCondition(andCond)
  node:AddNode(self:CreateNode("HealOther"))
  return node
end

function attackAggro()
  local node = self:CreateNode("Sequence")
  local haveAggro = self:CreateCondition("Filter")
  haveAggo:SetFilter(self:CreateFilter("SelectAggro"))
  node:AddCondition(haveAggro)
  node:AddNode(self:CreateNode("AttackSelection"))
  
  return node
end

function rezzAlly()
  local node = self:CreateNode("Sequence")
  local haveDeadAllies = self:CreateCondition("Filter")
  haveDeadAllies:SetFilter(self:CreateFilter("SelectDeadAllies"))
  node:AddNode(self:CreateNode("ResurrectSelection"))
end
