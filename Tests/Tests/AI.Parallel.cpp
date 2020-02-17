/**
 * Copyright 2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "stdafx.h"
#include <catch.hpp>

#include "AI.Mockup.h"
#include <abai/LuaLoader.h>
#include <abai/Registry.h>
#include <abai/Zone.h>
#include <abai/Agent.h>

TEST_CASE("Parallel")
{
    AI::TestRegistry reg;
    reg.Initialize();
    AI::LuaLoader loader(reg);
    const std::string script = R"lua(
function init(root)
    local nd = node("Parallel")
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

TEST_CASE("Parallel2")
{
    AI::TestRegistry reg;
    reg.Initialize();
    AI::LuaLoader loader(reg);
    const std::string script = R"lua(
function init(root)
    local nd = node("Parallel")
    nd:AddNode(node("TestAction"))
    nd:AddNode(node("RunningAction"))
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
    REQUIRE(agent->GetCurrentStatus() == AI::Node::Status::Running);
}

TEST_CASE("Parallel3")
{
    AI::TestRegistry reg;
    reg.Initialize();
    AI::LuaLoader loader(reg);
    const std::string script = R"lua(
function init(root)
    local nd = node("Parallel")
    nd:AddNode(node("TestAction"))
    nd:AddNode(node("Running2Action"))
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
    REQUIRE(agent->GetCurrentStatus() == AI::Node::Status::Running);
    // Running2Action needs 2 ticks to complete
    zone.Update(0);
    REQUIRE(agent->GetCurrentStatus() == AI::Node::Status::Finished);
}
