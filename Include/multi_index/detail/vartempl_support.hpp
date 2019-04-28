/* Copyright 2003-2013 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

/* Utilities for emulation of variadic template functions. Variadic packs are
 * replaced by lists of MULTI_INDEX_LIMIT_VARTEMPL_ARGS parameters:
 *
 * Forwarding emulated with Boost.Move. A template functions foo_imp
 * defined in such way accepts *exactly* MULTI_INDEX_LIMIT_VARTEMPL_ARGS
 * arguments: variable number of arguments is emulated by providing a set of
 * overloads foo forwarding to foo_impl with
 *
 *   MULTI_INDEX_OVERLOADS_TO_VARTEMPL
 *   MULTI_INDEX_OVERLOADS_TO_VARTEMPL_EXTRA_ARG (initial extra arg)
 *
 * which fill the extra args with multi_index::detail::noarg's.
 * multi_index::detail::vartempl_placement_new works the opposite
 * way: it acceps a full a pointer x to Value and a
 * MULTI_INDEX_FUNCTION_PARAM_PACK and forwards to
 * new(x) Value(args) where args is the argument pack after discarding
 * noarg's.
 *
 * Emulation decays to the real thing when the compiler supports variadic
 * templates and move semantics natively.
 */

#pragma once

#include <utility>

#define MULTI_INDEX_OVERLOADS_TO_VARTEMPL(                           \
  ret,name_from,name_to)                                             \
template<typename... Args> ret name_from(Args&&... args)             \
{                                                                    \
  return name_to(std::forward<Args>(args)...);                       \
}

#define MULTI_INDEX_OVERLOADS_TO_VARTEMPL_EXTRA_ARG(                 \
  ret,name_from,name_to,extra_arg_type,extra_arg_name)               \
template<typename... Args> ret name_from(                            \
  extra_arg_type extra_arg_name,Args&&... args)                      \
{                                                                    \
  return name_to(extra_arg_name,std::forward<Args>(args)...);        \
}

namespace multi_index{

namespace detail{

template<typename Value,typename... Args>
Value* vartempl_placement_new(Value*x,Args&&... args)
{
  return new(x) Value(std::forward<Args>(args)...);
}

} /* namespace multi_index::detail */

} /* namespace multi_index */
