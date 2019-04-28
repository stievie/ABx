/* Copyright 2003-2013 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

#include <brigand/functions/eval_if.hpp>
#include <brigand/functions/arithmetic/identity.hpp>
#include <multi_index/indexed_by.hpp>
#include <multi_index/detail/index_base.hpp>
#include <multi_index/detail/is_index_list.hpp>
#include <multi_index/detail/tuple_support.hpp>
#include <type_traits>

namespace multi_index{

namespace detail{

/* MPL machinery to construct a linear hierarchy of indices out of
 * a index list.
 */


template <typename IndexSpecifierMeta, typename SuperMeta>
struct next_index_layer
{
    using index_specifier = typename IndexSpecifierMeta::type;
    using type = typename index_specifier::template index_class<SuperMeta>::type;
};

template <int N,typename... Ts>
struct nth_layer_impl;


template <int N,typename V, typename A, typename... Ts>
struct nth_layer_impl<N, V, A, std::tuple<Ts...>>
{
  typedef std::tuple<Ts...> T;
  static const int length = std::tuple_size<T>::value;

  typedef typename brigand::eval_if_c<
    N==length,
    brigand::identity<index_base<V, indexed_by<Ts...>, A>>,
    next_index_layer<
      std::tuple_element<N, T>,
      nth_layer_impl<N+1, V, A, std::tuple<Ts...>>
    >
  >::type type;
};


template <int N, typename... Ts>
struct nth_layer;


template <int N,typename Value, typename Index, typename Allocator>
struct nth_layer<N, Value, Index, Allocator>
{
  using type = typename nth_layer_impl<N, Value, Allocator, Index>::type;
};


template <typename Value, typename IndexSpecifierList, typename Allocator>
struct multi_index_base_type: nth_layer<0,Value, IndexSpecifierList, Allocator>
{
  static_assert(detail::is_index_list<IndexSpecifierList>::value, "");
};

} /* namespace multi_index::detail */

} /* namespace multi_index */
