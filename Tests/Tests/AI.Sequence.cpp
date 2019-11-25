#include "stdafx.h"
#include <catch.hpp>

#include "AI.Mockup.h"
#include "LuaLoader.h"
#include "Registry.h"
#include "Zone.h"
#include "Agent.h"

TEST_CASE("Sequence")
{
    AI::TestRegistry reg;
    reg.Initialize();
    AI::LuaLoader loader(reg);
    const std::string script = R"lua(
function init(root)
    local nd = node("Sequence")
    nd:AddNode(node("TestAction"))
    root:AddNode(nd)
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
    AI::LuaLoader loader(reg);
    /*
    Sequence
        - Condition(Filter)              success 1 selected
            - Action
    */
    const std::string script = R"lua(
function init(root)
    local nd = node("Sequence")
    local haveAggro = condition("Filter")
    haveAggro:SetFilter(filter("SelectSelf"))
    nd:SetCondition(haveAggro)
    nd:AddNode(node("TestAction"))
    root:AddNode(nd)
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
    AI::LuaLoader loader(reg);
    /*
    Sequence
        - Condition(Filter)              fail because nothing is selected
            - Action
    */
    const std::string script = R"lua(
function init(root)
    local nd = node("Sequence")
    local haveAggro = condition("Filter")
    haveAggro:SetFilter(filter("SelectNothing"))
    nd:SetCondition(haveAggro)
    nd:AddNode(node("TestAction"))
    root:AddNode(nd)
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
