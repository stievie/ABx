#include "stdafx.h"
#include <catch.hpp>

#include "AI.Mockup.h"
#include "Loader.h"
#include "Registry.h"
#include "Zone.h"
#include "Agent.h"

TEST_CASE("Sequence")
{
    AI::TestRegistry reg;
    reg.Initialize();
    AI::Loader loader(reg);
    // I thought we call a lua function which adds nodes or whatever to the
    // root node, but I'm not sure...
    std::string script = R"lua(
function init(root)
    local node = self:CreateNode("Sequence")
    node:AddNode(self:CreateNode("TestAction"))
    root:AddNode(node)
end
)lua";
    auto root = loader.LoadString(script);
    REQUIRE(root);

    auto agent = std::make_shared<AI::Agent>(1);
    agent->SetBehavior(root);
    AI::Zone zone("test");
    zone.AddAgent(agent);
    zone.Update(0);
    REQUIRE(agent->GetCurrentStatus() == AI::Node::Status::Finished);
}

TEST_CASE("Sequence Filter condition")
{
    AI::TestRegistry reg;
    reg.Initialize();
    AI::Loader loader(reg);
    /*
    Sequence
        - Condition(Filter)              success 1 selected
            - Action
    */
    std::string script = R"lua(
function init(root)
    local node = self:CreateNode("Sequence")
    local haveAggro = self:CreateCondition("Filter")
    haveAggro:SetFilter(self:CreateFilter("SelectSelf"))
    node:SetCondition(haveAggro)
    node:AddNode(self:CreateNode("TestAction"))
    root:AddNode(node)
end
)lua";
    auto root = loader.LoadString(script);
    REQUIRE(root);

    auto agent = std::make_shared<AI::Agent>(1);
    agent->SetBehavior(root);
    AI::Zone zone("test");
    zone.AddAgent(agent);
    zone.Update(0);
    REQUIRE(agent->GetCurrentStatus() == AI::Node::Status::Finished);
}

TEST_CASE("Sequence Filter condition fail")
{
    AI::TestRegistry reg;
    reg.Initialize();
    AI::Loader loader(reg);
    /*
    Sequence
        - Condition(Filter)              fail because nothing is selected
            - Action
    */
    std::string script = R"lua(
function init(root)
    local node = self:CreateNode("Sequence")
    local haveAggro = self:CreateCondition("Filter")
    haveAggro:SetFilter(self:CreateFilter("SelectNothing"))
    node:SetCondition(haveAggro)
    node:AddNode(self:CreateNode("TestAction"))
    root:AddNode(node)
end
)lua";
    auto root = loader.LoadString(script);
    REQUIRE(root);

    auto agent = std::make_shared<AI::Agent>(1);
    agent->SetBehavior(root);
    AI::Zone zone("test");
    zone.AddAgent(agent);
    zone.Update(0);
    REQUIRE(agent->GetCurrentStatus() != AI::Node::Status::Finished);
}
