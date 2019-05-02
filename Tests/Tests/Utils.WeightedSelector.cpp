#include "stdafx.h"
#include <catch.hpp>

#include "WeightedSelector.h"

TEST_CASE("WeightedSelector")
{
    Utils::WeightedSelector<std::string> ws;
    ws.Add("Test1", 0.5f);
    ws.Add("Test2", 0.2f);
    ws.Add("Test3", 0.15f);
    ws.Add("Test4", 0.1f);
    ws.Add("Test5", 0.05f);
    ws.Add("Test6", 0.05f);
    ws.Add("Test7", 0.05f);
    ws.Add("Test8", 0.05f);
    ws.Add("Test9", 0.05f);
    ws.Add("Test10", 0.05f);
    ws.Update();

    std::string s = ws.Get(0.3f, 0.25f);
    REQUIRE(s == "Test4");
}
