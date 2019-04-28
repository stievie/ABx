/* Copyright 2003-2015 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

namespace multi_index{

/* ordered_index specifiers */

template <typename T1, typename... Ts>
struct ordered_unique;

template<typename T1, typename... Ts>
struct ordered_non_unique;

} /* namespace multi_index */
