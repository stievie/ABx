/* Copyright 2003-2013 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

#include <brigand/functions/logical/or.hpp>
#include <brigand/types/bool.hpp>
#include <type_traits>

namespace multi_index{

namespace detail{

/* determines whether an index type has a given tag in its tag list */


template <typename Tag, typename... Ts>
struct has_tag_impl;


template <typename Tag>
struct has_tag_impl<Tag> : brigand::false_type {};


template <typename Tag, typename T, typename... Ts>
struct has_tag_impl<Tag, T, Ts...> : brigand::or_<std::is_same<Tag, T>, has_tag_impl<Tag, Ts...>> {};


template <typename Tag, typename... Ts>
struct has_tag;


template <typename Tag, typename... Ts, template <typename...> class C>
struct has_tag<Tag, C<Ts...>>
{
    using type = has_tag_impl<Tag, Ts...>;
};

} /* namespace multi_index::detail */

} /* namespace multi_index */
