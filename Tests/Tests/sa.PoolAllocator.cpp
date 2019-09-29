#include "stdafx.h"
#include <catch.hpp>
#define DEBUG_POOLALLOCATOR
#include <sa/PoolAllocator.h>
#include <memory>
#include <vector>
#include <sa/SharedPtr.h>

class _A
{
public:
    int x;
};

class A : public _A
{
public:
    uint8_t buff[4096];
};

class B
{
public:
    uint8_t buff[4096];
};

using AAllocator = sa::PoolAllocator<A, sizeof(A)>;
static AAllocator gAllocator(sizeof(A) * 1024);

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

namespace sa {

template <>
struct DefaultDelete<A>
{
    DefaultDelete() = default;
    void operator()(A* p) const noexcept
    {
        gAllocator.deallocate(p, 1);
    }
};

template <>
inline SharedPtr<A> MakeShared()
{
    auto* ptr = gAllocator.allocate(1, nullptr);
    return sa::SharedPtr<A>(ptr);
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

TEST_CASE("SharedPtr PoolAllocator")
{
    auto previnfo = gAllocator.GetInfo();
    {
        std::vector<sa::SharedPtr<A>> bs;
        {
            for (int i = 0; i < 100; ++i)
            {
                bs.push_back(sa::MakeShared<A>());
            }
        }
        auto currinfo = gAllocator.GetInfo();
        REQUIRE(currinfo.allocs == previnfo.allocs + 100);
        REQUIRE(currinfo.frees == previnfo.frees);
    }
    auto currinfo = gAllocator.GetInfo();
    REQUIRE(currinfo.current == 0);
}
