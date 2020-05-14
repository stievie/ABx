#include "stdafx.h"
#include <catch.hpp>

#include <sa/Result.h>
#include <string>
#include <sstream>

TEST_CASE("Result construct Ok")
{
    sa::Result<int, std::string> res(1);
    REQUIRE(res.IsOk());
    REQUIRE(res == 1);
    REQUIRE(res);
}

TEST_CASE("Result construct Error")
{
    sa::Result<int, std::string> res(std::string("Some Error"));
    REQUIRE(!res.IsOk());
    REQUIRE(res.IsError());
    REQUIRE(res == std::string("Some Error"));
    std::string error = static_cast<std::string>(res);
    REQUIRE(error == "Some Error");
}

TEST_CASE("Result assign Ok")
{
    sa::Result<int, std::string> res;
    res = 1;
    REQUIRE(res.IsOk());
    REQUIRE(res == 1);
    REQUIRE(res);
}

TEST_CASE("Result assign Error")
{
    sa::Result<int, std::string> res;
    // Need explicit instantiation, otherwise it would not pickup the right assignment operator.
    // Too much implicitness: const char* (1)-> std::string (2)-> Error<std::string>
    res = std::string("Some Error");
    REQUIRE(!res.IsOk());
    REQUIRE(res == std::string("Some Error"));
}

TEST_CASE("Result Ok cast")
{
    sa::Result<int, std::string> res(1);
    REQUIRE(res.IsOk());
    REQUIRE(res == 1);
    int ok = static_cast<int>(res);
    REQUIRE(ok == 1);
    int ok2 = static_cast<decltype(res)::OkType>(res);
    REQUIRE(ok2 == 1);
}

TEST_CASE("Result Error cast")
{
    sa::Result<int, std::string> res(std::string("Some Error"));
    REQUIRE(!res.IsOk());
    REQUIRE(res.IsError());
    std::string error = static_cast<std::string>(res);
    REQUIRE(error == "Some Error");
    std::string error2 = static_cast<decltype(res)::ErrorType>(res);
    REQUIRE(error2 == "Some Error");
}

TEST_CASE("Result Ok == operator")
{
    sa::Result<int, std::string> res(1);
    sa::Result<int, std::string> res2(1);
    REQUIRE(res == res2);
    REQUIRE(res == 1);
}

TEST_CASE("Result Error == operator")
{
    sa::Result<int, std::string> res(std::string("Some Error"));
    sa::Result<int, std::string> res2(std::string("Some Error"));
    REQUIRE(res == res2);
}

TEST_CASE("Result Ok << operator")
{
    sa::Result<int, std::string> res(1);
    std::stringstream ss;
    ss << res;
    REQUIRE(ss.str() == "1");
}

TEST_CASE("Result Error << operator")
{
    sa::Result<int, std::string> res(std::string("Some Error"));
    std::stringstream ss;
    ss << res;
    REQUIRE(ss.str() == "Some Error");
}

TEST_CASE("Result GetOk()")
{
    sa::Result<int, std::string> res(1);
    auto ok = res.GetOk();
    REQUIRE(ok.has_value());
    REQUIRE(ok.value() == 1);
    auto error = res.GetError();
    REQUIRE(!error.has_value());
}

TEST_CASE("Result GetError()")
{
    sa::Result<int, std::string> res(std::string("Some Error"));
    auto ok = res.GetOk();
    REQUIRE(!ok.has_value());
    auto error = res.GetError();
    REQUIRE(error.has_value());
    REQUIRE(error.value() == "Some Error");
}

TEST_CASE("Result Result::Ok()")
{
    auto res = sa::Result<int, std::string>::Ok(1);
    auto ok = res.GetOk();
    REQUIRE(ok.has_value());
    REQUIRE(ok.value() == 1);
    auto error = res.GetError();
    REQUIRE(!error.has_value());
}

TEST_CASE("Result Result::Error()")
{
    auto res = sa::Result<int, std::string>::Error("Some Error");
    auto ok = res.GetOk();
    REQUIRE(!ok.has_value());
    auto error = res.GetError();
    REQUIRE(error.has_value());
    REQUIRE(error.value() == "Some Error");
}
