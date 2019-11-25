#include "stdafx.h"
#include <catch.hpp>

#include "LuaLoader.h"
#include "Registry.h"

TEST_CASE("Load")
{
    AI::Registry reg;
    reg.Initialize();
    AI::LuaLoader loader(reg);
    // I thought we call a lua function which adds nodes or whatever to the
    // root node, but I'm not sure...
    std::string script = R"lua(
function init(root)
    local nd = node("Parallel", {1, "xx"})
    root:AddNode(nd)
end
)lua";
    auto root = loader.LoadString(script);
    REQUIRE(root);
}
