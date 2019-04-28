/* Copyright 2017 Alex Huszagh.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

#include <brigand/adapted/tuple.hpp>
#include <brigand/algorithms/transform.hpp>
#include <brigand/functions/eval_if.hpp>
#include <brigand/functions/arithmetic/identity.hpp>
#include <multi_index/tuple.hpp>
#include <tuple>


namespace multi_index
{
/* fold tuple */

template <typename T, typename R>
struct to_stdtuple;

template <typename... Ts, typename T>
struct to_stdtuple<std::tuple< Ts... >, T>
{
    typedef std::tuple<Ts..., T> type;
};

/* generic-use */

template <typename T>
using get_type = typename T::type;

/* build a tuple of size N of homogenous types */

template <int, typename ... Ts>
struct tuple_builder_impl;

template <int N, typename T, typename ... Ts>
struct tuple_builder_impl<N, T, Ts...>
{
    using type = typename brigand::eval_if_c<
        N == 0,
        brigand::identity<tuple<Ts...>>,
        tuple_builder_impl<N - 1, T, T, Ts...>
    >::type;
};

template <int N, typename T>
using tuple_builder = typename tuple_builder_impl<N, T>::type;


/* build a tuple of composite, indexed types */

template <int, int, template <typename, int> class C, typename... Ts>
struct indexed_tuple_builder_impl;

template <int I, int N, template <typename, int> class C, typename T, typename ... Ts>
struct indexed_tuple_builder_impl<I, N, C, T, Ts...>
{
    using type = typename brigand::eval_if_c<
        I == 0,
        brigand::identity<tuple<Ts...>>,
        indexed_tuple_builder_impl<I - 1, N, C, T, C<T, I-1>, Ts...>
    >::type;
};

template <int I, template <typename, int> class C, typename T>
using indexed_tuple_builder = typename indexed_tuple_builder_impl<I, I, C, T>::type;

/* extract the type from each element in a tuple */

template <typename... Ts>
using tuple_wrapper = tuple<Ts...>;

template <typename L>
using as_tuple = brigand::wrap<L, tuple_wrapper>;

template <typename... Ts>
struct extract_tuple_element_impl;

template <typename... Ts>
struct extract_tuple_element_impl<tuple<Ts...>>
{
    using get_t = brigand::bind<get_type, brigand::_1>;
    using list = brigand::transform<brigand::list<Ts...>, get_t>;
    using type = as_tuple<list>;
};

template <typename T>
using extract_tuple_element = typename extract_tuple_element_impl<T>::type;

/* convert tuple of composite types to another type */

template <template <typename...> class C, typename ... Ts>
struct tuple_as_class_impl;

template <template <typename...> class C, typename... Ts>
struct tuple_as_class_impl<C, tuple<Ts...>>
{
    using type = C<Ts...>;
};

template <template <typename...> class C, typename ... Ts>
using tuple_as_class = typename tuple_as_class_impl<C, Ts...>::type;

}   /* multi_index */
