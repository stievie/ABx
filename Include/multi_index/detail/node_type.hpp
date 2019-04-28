/* Copyright 2003-2013 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

#include <brigand/algorithms/fold.hpp>
#include <multi_index_container_fwd.hpp>
#include <multi_index/detail/header_holder.hpp>
#include <multi_index/detail/index_node_base.hpp>
#include <multi_index/detail/is_index_list.hpp>
#include <multi_index/detail/tuple_support.hpp>

namespace multi_index{

namespace detail{

/* MPL machinery to construct the internal node type associated to an
 * index list.
 */

struct index_node_applier
{
  template<typename IndexSpecifierIterator,typename Super>
  struct apply
  {
    using index_specifier = typename IndexSpecifierIterator::type;
    typedef typename index_specifier::template node_class<Super>::type type;
  };
};

template<typename IndexSpecifier,typename Super>
using next_index_node = typename IndexSpecifier::template node_class<Super>::type;

template <typename... Ts>
struct multi_index_node_type_impl;

template <typename Value, typename Allocator, typename... Ts>
struct multi_index_node_type_impl<Value, Allocator, std::tuple<Ts...>>
{
  using type = brigand::reverse_fold<
    brigand::list<Ts...>,
    index_node_base<Value,Allocator>,
    brigand::bind<next_index_node, brigand::_2, brigand::_1>
  >;
};


template<typename Value,typename IndexSpecifierList,typename Allocator>
struct multi_index_node_type
{
  static_assert(detail::is_index_list<IndexSpecifierList>::value, "");

  using impl = multi_index_node_type_impl<Value, Allocator, IndexSpecifierList>;
  using type = typename impl::type;
};

} /* namespace multi_index::detail */

} /* namespace multi_index */
