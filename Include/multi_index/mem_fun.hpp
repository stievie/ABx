/* Copyright 2003-2015 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

#include <functional>
#include <type_traits>

namespace multi_index{

/* mem_fun implements a read-only key extractor based on a given non-const
 * member function of a class.
 * const_mem_fun does the same for const member functions.
 * Additionally, mem_fun  and const_mem_fun are overloaded to support
 * referece_wrappers of T and "chained pointers" to T's. By chained pointer
 * to T we  mean a type P such that, given a p of Type P
 *   *...n...*x is convertible to T&, for some n>=1.
 * Examples of chained pointers are raw and smart pointers, iterators and
 * arbitrary combinations of these (vg. T** or unique_ptr<T*>.)
 */

template<class Class,typename Type,Type (Class::*PtrToMemberFunction)()const>
struct const_mem_fun
{
  typedef typename std::remove_reference<Type>::type result_type;

  template<typename ChainedPtr>

  typename std::enable_if<
    !std::is_convertible<const ChainedPtr&,const Class&>::value,Type>::type

  operator()(const ChainedPtr& x)const
  {
    return operator()(*x);
  }

  Type operator()(const Class& x)const
  {
    return (x.*PtrToMemberFunction)();
  }

  Type operator()(const std::reference_wrapper<const Class>& x)const
  {
    return operator()(x.get());
  }

  Type operator()(const std::reference_wrapper<Class>& x)const
  {
    return operator()(x.get());
  }
};

template<class Class,typename Type,Type (Class::*PtrToMemberFunction)()>
struct mem_fun
{
  typedef typename std::remove_reference<Type>::type result_type;

  template<typename ChainedPtr>

  typename std::enable_if<
    !std::is_convertible<ChainedPtr&,Class&>::value,Type>::type

  operator()(const ChainedPtr& x)const
  {
    return operator()(*x);
  }

  Type operator()(Class& x)const
  {
    return (x.*PtrToMemberFunction)();
  }

  Type operator()(const std::reference_wrapper<Class>& x)const
  {
    return operator()(x.get());
  }
};

/* MSVC++ 6.0 has problems with const member functions as non-type template
 * parameters, somehow it takes them as non-const. const_mem_fun_explicit
 * workarounds this deficiency by accepting an extra type parameter that
 * specifies the signature of the member function. The workaround was found at:
 *   Daniel, C.:"Re: weird typedef problem in VC",
 *   news:microsoft.public.vc.language, 21st nov 2002,
 *   http://groups.google.com/groups?
 *     hl=en&lr=&ie=UTF-8&selm=ukwvg3O0BHA.1512%40tkmsftngp05
 *
 * MSVC++ 6.0 support has been dropped and [const_]mem_fun_explicit is
 * deprecated.
 */

template<
  class Class,typename Type,
  typename PtrToMemberFunctionType,PtrToMemberFunctionType PtrToMemberFunction>
struct const_mem_fun_explicit
{
  typedef typename std::remove_reference<Type>::type result_type;

  template<typename ChainedPtr>

  typename std::enable_if<
    !std::is_convertible<const ChainedPtr&,const Class&>::value,Type>::type

  operator()(const ChainedPtr& x)const
  {
    return operator()(*x);
  }

  Type operator()(const Class& x)const
  {
    return (x.*PtrToMemberFunction)();
  }

  Type operator()(const std::reference_wrapper<const Class>& x)const
  {
    return operator()(x.get());
  }

  Type operator()(const std::reference_wrapper<Class>& x)const
  {
    return operator()(x.get());
  }
};

template<
  class Class,typename Type,
  typename PtrToMemberFunctionType,PtrToMemberFunctionType PtrToMemberFunction>
struct mem_fun_explicit
{
  typedef typename std::remove_reference<Type>::type result_type;

  template<typename ChainedPtr>

  typename std::enable_if<
    !std::is_convertible<ChainedPtr&,Class&>::value,Type>::type

  operator()(const ChainedPtr& x)const
  {
    return operator()(*x);
  }

  Type operator()(Class& x)const
  {
    return (x.*PtrToMemberFunction)();
  }

  Type operator()(const std::reference_wrapper<Class>& x)const
  {
    return operator()(x.get());
  }
};

} /* namespace multi_index */
