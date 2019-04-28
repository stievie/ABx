/* Copyright 2003-2013 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

namespace multi_index{

namespace detail{

/* Used to mark a special ctor variant that copies the internal objects of
 * a container but not its elements.
 */

struct do_not_copy_elements_tag{};

} /* namespace multi_index::detail */

} /* namespace multi_index */
