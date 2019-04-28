/* Copyright 2003-2014 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

#include <brigand/functions/if.hpp>
#include <multi_index/tuple.hpp>
#include <tuple>

namespace multi_index{

namespace detail{

/* std::tuple wrapper providing the cons-based interface of multi_index::tuple for
 * composite_key interoperability.
 */

template<typename StdTuple,std::size_t N>
struct cons_stdtuple;

struct cons_stdtuple_ctor_terminal
{
  typedef multi_index::tuples::null_type result_type;

  template<typename StdTuple>
  static result_type create(const StdTuple&)
  {
    return multi_index::tuples::null_type();
  }
};

template<typename StdTuple,std::size_t N>
struct cons_stdtuple_ctor_normal
{
  typedef cons_stdtuple<StdTuple,N> result_type;

  static result_type create(const StdTuple& t)
  {
    return result_type(t);
  }
};

template<typename StdTuple,std::size_t N=0>
struct cons_stdtuple_ctor:
  brigand::if_c<
    N<std::tuple_size<StdTuple>::value,
    cons_stdtuple_ctor_normal<StdTuple,N>,
    cons_stdtuple_ctor_terminal
  >::type
{};

template<typename StdTuple,std::size_t N>
struct cons_stdtuple
{
  typedef typename std::tuple_element<N,StdTuple>::type head_type;
  typedef cons_stdtuple_ctor<StdTuple,N+1>              tail_ctor;
  typedef typename tail_ctor::result_type               tail_type;

  cons_stdtuple(const StdTuple& t_):t(t_){}

  const head_type& get_head()const{return std::get<N>(t);}
  tail_type get_tail()const{return tail_ctor::create(t);}

  const StdTuple& t;
};

template<typename StdTuple>
typename cons_stdtuple_ctor<StdTuple>::result_type
make_cons_stdtuple(const StdTuple& t)
{
  return cons_stdtuple_ctor<StdTuple>::create(t);
}

} /* namespace multi_index::detail */

} /* namespace multi_index */
