#include "stdafx.h"
#include <catch.hpp>

#include "Matrix4.h"

TEST_CASE("Matrix4 Construct")
{
    SECTION("Basic")
    {
        Math::Matrix4 mat;
        XMath::XMMATRIX xmat = (XMath::XMMATRIX)mat;
        Math::Matrix4 mat2(xmat);
        REQUIRE(mat.Translation() == mat2.Translation());
        REQUIRE(mat.Scaling() == mat2.Scaling());
        REQUIRE(mat.Rotation() == mat2.Rotation());
    }
}