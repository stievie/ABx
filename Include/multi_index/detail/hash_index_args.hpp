/* Copyright 2003-2013 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

#include <brigand/functions/eval_if.hpp>
#include <multi_index/config.hpp>
#include <multi_index/tag.hpp>
#include <functional>
#include <type_traits>

namespace multi_index{

namespace detail{

/* Hashed index specifiers can be instantiated in two forms:
 *
 *   (hashed_unique|hashed_non_unique)<
 *     KeyFromValue,
 *     Hash=hash<KeyFromValue::result_type>,
 *     Pred=std::equal_to<KeyFromValue::result_type> >
 *   (hashed_unique|hashed_non_unique)<
 *     TagList,
 *     KeyFromValue,
 *     Hash=hash<KeyFromValue::result_type>,
 *     Pred=std::equal_to<KeyFromValue::result_type> >
 *
 * hashed_index_args implements the machinery to accept this
 * argument-dependent polymorphism.
 */

template<typename T>
using index_args_default_hash = MULTI_INDEX_HASH<typename T::result_type>;

template<typename T>
using index_args_default_pred = MULTI_INDEX_EQUAL_TO<typename T::result_type>;


template <typename T1, typename T2, typename T3 = index_args_default_hash<T2>, typename T4 = index_args_default_pred<T2>>
struct hashed_index_args_full_form_impl
{
  typedef T1 tag_list_type;
  typedef T2 key_from_value_type;
  typedef T3 hash_type;
  typedef T4 pred_type;
};

template <typename T1, typename T2 = index_args_default_hash<T1>, typename T3 = index_args_default_pred<T1>>
struct hashed_index_args_short_form_impl
{
  typedef tag<> tag_list_type;
  typedef T1 key_from_value_type;
  typedef T2 hash_type;
  typedef T3 pred_type;
};


template <typename T1, typename... Ts>
struct hashed_index_args_full_form
{
  typedef hashed_index_args_full_form_impl<T1, Ts...> type;
};


template <typename T1, typename... Ts>
struct hashed_index_args_short_form
{
  typedef hashed_index_args_short_form_impl<T1, Ts...> type;
};


template <typename T1, typename... Ts>
struct hashed_index_args: brigand::eval_if<
    is_tag<T1>,
    hashed_index_args_full_form<T1, Ts...>,
    hashed_index_args_short_form<T1, Ts...>
  >::type
{};

} /* namespace multi_index::detail */

} /* namespace multi_index */
