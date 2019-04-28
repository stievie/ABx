/* Copyright 2003-2014 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

#include <brigand/functions/logical/or.hpp>
#include <brigand/functions/logical/not.hpp>
#include <brigand/types/bool.hpp>
#include <multi_index/detail/function_traits.hpp>
#include <type_traits>
#include <utility>

namespace multi_index{

namespace detail{

/* Metafunction that checks if f(arg,arg2) executes without argument type
 * conversion. By default (i.e. when it cannot be determined) it evaluates to
 * true.
 */

template<typename F,typename Arg1,typename Arg2,typename=void>
struct is_transparent:brigand::true_type{};

struct not_is_transparent_result_type{};

template<typename F,typename Arg1,typename Arg2>
struct is_transparent_class_helper:F
{
  using F::operator();
  template<typename T,typename Q>
  not_is_transparent_result_type operator()(const T&,const Q&)const;
};

template<typename F,typename Arg1,typename Arg2,typename=void>
struct is_transparent_class:brigand::true_type{};

template<typename F,typename Arg1,typename Arg2>
struct is_transparent_class<
  F,Arg1,Arg2,
  typename std::enable_if<
    std::is_same<
      decltype(
        std::declval<const is_transparent_class_helper<F,Arg1,Arg2> >()(
          std::declval<const Arg1&>(),std::declval<const Arg2&>())
      ),
      not_is_transparent_result_type
    >::value
  >::type
>:brigand::false_type{};

template<typename F,typename Arg1,typename Arg2>
struct is_transparent<
  F,Arg1,Arg2,
  typename std::enable_if<
#if __cplusplus >= 201402L
    std::is_class<F>::value && !std::is_final<F>::value
#else
    std::is_class<F>::value
#endif
  >::type
>:is_transparent_class<F,Arg1,Arg2>{};

template<typename F,typename Arg1,typename Arg2,typename=void>
struct is_transparent_function:brigand::true_type{};

template<typename F,typename Arg1,typename Arg2>
struct is_transparent_function<
  F,Arg1,Arg2,
  typename std::enable_if<
    brigand::or_<
      brigand::not_<brigand::or_<
        std::is_same<typename function_traits<F>::arg1_type,const Arg1&>,
        std::is_same<typename function_traits<F>::arg1_type,Arg1>
      > >,
      brigand::not_<brigand::or_<
        std::is_same<typename function_traits<F>::arg2_type,const Arg2&>,
        std::is_same<typename function_traits<F>::arg2_type,Arg2>
      > >
    >::value
  >::type
>:brigand::false_type{};

template<typename F,typename Arg1,typename Arg2>
struct is_transparent<
  F,Arg1,Arg2,
  typename std::enable_if<
    std::is_function<typename std::remove_pointer<F>::type>::value
  >::type
>:is_transparent_function<typename std::remove_pointer<F>::type,Arg1,Arg2>{};

} /* namespace multi_index::detail */

} /* namespace multi_index */
