/* Copyright 2003-2013 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

#include <brigand/algorithms/transform.hpp>
#include <brigand/functions/if.hpp>
#include <brigand/functions/logical/not.hpp>
#include <brigand/sequences/list.hpp>
#include <brigand/sequences/set.hpp>
#include <brigand/sequences/size.hpp>

namespace multi_index{

namespace detail{

/* no_duplicate_tags check at compile-time that a tag list
 * has no duplicate tags.
 * The algorithm deserves some explanation: tags
 * are sequentially inserted into a mpl::set if they were
 * not already present. Due to the magic of mpl::set
 * (mpl::has_key is contant time), this operation takes linear
 * time, and even MSVC++ 6.5 handles it gracefully (other obvious
 * solutions are quadratic.)
 */


template <typename Index>
struct index_to_tag
{
  using type = typename Index::tag_list;
};


template <typename List>
struct index_list_to_tag_list
{
  using type = brigand::transform<List, index_to_tag<brigand::_1>>;
};


template <typename T>
struct is_not_empty
{
  using type = brigand::not_<brigand::size<T>>;
};


template <typename... Tags>
struct no_duplicate_tags;


template <typename... Tags, template <typename...> class C>
struct no_duplicate_tags<C<Tags...>>
{
  using tag_list = brigand::list<Tags...>;
  using tag_set = brigand::as_set<tag_list>;

  static const bool value = brigand::size<tag_list>::value == brigand::size<tag_set>::value;
};


template <typename... Indices>
struct no_duplicate_tags_in_index_list;


template <typename... Indices, template <typename...> class C>
struct no_duplicate_tags_in_index_list<C<Indices...>>
{
  using list = brigand::list<Indices...>;
  using not_empty = brigand::find<list, is_not_empty<brigand::_1>>;
  using tag_list = typename index_list_to_tag_list<not_empty>::type;

  static const bool value = no_duplicate_tags<tag_list>::value;
};

} /* namespace multi_index::detail */

} /* namespace multi_index */
