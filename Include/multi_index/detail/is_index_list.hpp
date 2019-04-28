/* Copyright 2003-2013 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

#include <tuple>


namespace multi_index{

namespace detail{

template <typename T>
struct is_index_list
{
  static const bool non_empty = std::tuple_size<T>::value != 0;
  static const bool value = non_empty;
};

} /* namespace multi_index::detail */

} /* namespace multi_index */
