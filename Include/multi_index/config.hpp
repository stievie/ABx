/* Copyright 2017 Alex Huszagh.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

// Change the default hash function
#ifndef MULTI_INDEX_HASH
#   define MULTI_INDEX_HASH std::hash
#endif

// Change the default equal_to function
#ifndef MULTI_INDEX_EQUAL_TO
#   define MULTI_INDEX_EQUAL_TO std::equal_to
#endif

// Change the default less function
#ifndef MULTI_INDEX_LESS
#   define MULTI_INDEX_LESS std::less
#endif

// Change the default greater function
#ifndef MULTI_INDEX_GREATER
#   define MULTI_INDEX_GREATER std::greater
#endif
