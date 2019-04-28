/* Copyright 2003-2013 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

#include <brigand/sequences/list.hpp>
#include <multi_index/detail/is_base_and_derived.hpp>
#include <multi_index/detail/no_duplicate_tags.hpp>

namespace multi_index{

namespace detail{

struct tag_marker{};

template<typename T>
struct is_tag: is_base_and_derived<tag_marker, T>
{};

} /* namespace multi_index::detail */

template<typename... Ts>
struct tag: private detail::tag_marker
{
  using type = brigand::list<Ts...>;
  static_assert(detail::no_duplicate_tags<type>::value, "");
};

} /* namespace multi_index */
