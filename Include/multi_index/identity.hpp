/* Copyright 2003-2015 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

#include <multi_index/identity_fwd.hpp>
#include <functional>
#include <type_traits>

namespace multi_index{

namespace detail{

/* identity is a do-nothing key extractor that returns the [const] Type&
 * object passed.
 * Additionally, identity is overloaded to support referece_wrappers
 * of Type and "chained pointers" to Type's. By chained pointer to Type we
 * mean a  type  P such that, given a p of type P
 *   *...n...*x is convertible to Type&, for some n>=1.
 * Examples of chained pointers are raw and smart pointers, iterators and
 * arbitrary combinations of these (vg. Type** or unique_ptr<Type*>.)
 */

template<typename Type>
struct const_identity_base
{
  typedef Type result_type;

  template<typename ChainedPtr>

  typename std::enable_if<!std::is_convertible<const ChainedPtr&,Type&>::value,Type&>::type

  operator()(const ChainedPtr& x)const
  {
    return operator()(*x);
  }

  Type& operator()(Type& x)const
  {
    return x;
  }

  Type& operator()(const std::reference_wrapper<Type>& x)const
  {
    return x.get();
  }

  Type& operator()(
    const std::reference_wrapper<typename std::remove_const<Type>::type>& x

  )const
  {
    return x.get();
  }
};

template<typename Type>
struct non_const_identity_base
{
  typedef Type result_type;

  /* templatized for pointer-like types */

  template<typename ChainedPtr>

  typename std::enable_if<
    !std::is_convertible<const ChainedPtr&,const Type&>::value,Type&>::type

  operator()(const ChainedPtr& x)const
  {
    return operator()(*x);
  }

  const Type& operator()(const Type& x)const
  {
    return x;
  }

  Type& operator()(Type& x)const
  {
    return x;
  }

  const Type& operator()(const std::reference_wrapper<const Type>& x)const
  {
    return x.get();
  }

  Type& operator()(const std::reference_wrapper<Type>& x)const
  {
    return x.get();
  }
};

} /* namespace multi_index::detail */

template<class Type>
struct identity:
  std::conditional<
    std::is_const<Type>::value,
    detail::const_identity_base<Type>,detail::non_const_identity_base<Type>
  >::type
{
};

} /* namespace multi_index */
