#include "stdafx.h"
#include <catch.hpp>
#define DEBUG_POOLALLOCATOR
#include <sa/PoolAllocator.h>
#include <memory>


class A
{
public:
    uint8_t buff[4096];
};

class B
{
public:
    uint8_t buff[4096];
};

using Allocator = sa::PoolAllocator<A, sizeof(A) * 1024, sizeof(A)>;
static Allocator gAllocator;

namespace std {

template <>
struct default_delete<A> {
    default_delete() = default;
    void operator()(A* p) const noexcept { gAllocator.deallocate(p, 1); }
};

template <>
inline unique_ptr<A> make_unique<A>()
{
    auto* ptr = gAllocator.allocate(1, nullptr);
    return std::unique_ptr<A>(ptr);
}

}

TEST_CASE("PoolAllocator")
{
    SECTION("Alloc/Free unique_ptr")
    {

        for (unsigned i = 0; i < 100; ++i)
        {
            auto a = std::make_unique<A>();
            REQUIRE(a);
            REQUIRE(sizeof(*a) == sizeof(A));
        }
        auto info = gAllocator.GetInfo();
        REQUIRE(info.allocs == 100);
        REQUIRE(info.frees == 100);
        // Obviously must be zero because it's calculated
        REQUIRE(info.current == 0);
    }
    SECTION("make_unique unique_ptr")
    {

        for (unsigned i = 0; i < 100; ++i)
        {
            auto b = std::make_unique<B>();
            REQUIRE(b);
            REQUIRE(sizeof(*b) == sizeof(B));
        }
    }
}
