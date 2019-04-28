/* Copyright 2003-2013 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

#include <brigand/functions/eval_if.hpp>
#include <multi_index/tag.hpp>
#include <functional>

namespace multi_index{

namespace detail{

/* Oredered index specifiers can be instantiated in two forms:
 *
 *   (ordered_unique|ordered_non_unique)<
 *     KeyFromValue,Compare=std::less<KeyFromValue::result_type> >
 *   (ordered_unique|ordered_non_unique)<
 *     TagList,KeyFromValue,Compare=std::less<KeyFromValue::result_type> >
 *
 * index_args implements the machinery to accept this argument-dependent
 * polymorphism.
 */

template<typename T>
using index_args_default_compare = MULTI_INDEX_LESS<typename T::result_type>;

template <typename T1, typename T2, typename T3 = index_args_default_compare<T2>>
struct ordered_index_args_full_form_impl
{
  typedef T1 tag_list_type;
  typedef T2 key_from_value_type;
  typedef T3 compare_type;
};

template <typename T1, typename T2 = index_args_default_compare<T1>>
struct ordered_index_args_short_form_impl
{
  typedef tag<> tag_list_type;
  typedef T1 key_from_value_type;
  typedef T2 compare_type;
};


template <typename T1, typename... Ts>
struct ordered_index_args_full_form
{
  typedef ordered_index_args_full_form_impl<T1, Ts...> type;
};


template <typename T1, typename... Ts>
struct ordered_index_args_short_form
{
  typedef ordered_index_args_short_form_impl<T1, Ts...> type;
};


template <typename T1, typename... Ts>
struct ordered_index_args: brigand::eval_if<
    is_tag<T1>,
    ordered_index_args_full_form<T1, Ts...>,
    ordered_index_args_short_form<T1, Ts...>
  >::type
{};

} /* namespace multi_index::detail */

} /* namespace multi_index */
