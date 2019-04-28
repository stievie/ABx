
//  (C) Copyright Rani Sharoni 2003.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/type_traits for most recent version including documentation.

#pragma once

#include <type_traits>

namespace multi_index {

template <typename Base, typename Derived>
struct is_base_and_derived
{
  static constexpr bool value = std::is_base_of<Base, Derived>::value && !std::is_same<Base, Derived>::value;
};

} // namespace multi_index
