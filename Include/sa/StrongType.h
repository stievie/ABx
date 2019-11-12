#pragma once

#include <type_traits>

namespace sa {

// To be used with fundamental types
// using int_type1 = sa::StrongType<int, struct IntTag1>;
// using int_type2 = sa::StrongType<int, struct IntTag2>;

template <typename T, typename Tag>
struct StrongType
{
    static_assert(std::is_fundamental<T>::value, "T must be a fundamental type");
    StrongType() = default;
    StrongType(T value) :
        value(value)
    { }
    operator T () const { return value; }
    T value;
};

// To be used with structs and classes
template <typename T, typename Tag>
struct StrongClass : public T
{
    static_assert(std::is_class<T>::value, "T must be a class or struct type");
};

}
