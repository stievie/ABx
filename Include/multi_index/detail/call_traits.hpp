//  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).
//
//  See http://www.boost.org/libs/utility for most recent version including documentation.

// call_traits: defines typedefs for function usage
// (see libs/utility/call_traits.htm)

/* Release notes:
   23rd July 2000:
      Fixed array specialization. (JM)
      Added Borland specific fixes for reference types
      (issue raised by Steve Cleary).
*/

#pragma once

#include <cstddef>
#include <type_traits>

namespace multi_index{
namespace detail{

template <typename T, bool small_>
struct ct_imp2
{
   typedef const T& param_type;
};

template <typename T>
struct ct_imp2<T, true>
{
   typedef const T param_type;
};

template <typename T, bool isp, bool b1, bool b2>
struct ct_imp
{
   typedef const T& param_type;
};

template <typename T, bool isp, bool b2>
struct ct_imp<T, isp, true, b2>
{
   typedef typename ct_imp2<T, sizeof(T) <= sizeof(void*)>::param_type param_type;
};

template <typename T, bool isp, bool b1>
struct ct_imp<T, isp, b1, true>
{
   typedef typename ct_imp2<T, sizeof(T) <= sizeof(void*)>::param_type param_type;
};

template <typename T, bool b1, bool b2>
struct ct_imp<T, true, b1, b2>
{
   typedef const T param_type;
};

}  /* detail */

template <typename T>
struct call_traits
{
public:
   typedef T value_type;
   typedef T& reference;
   typedef const T& const_reference;
   //
   // C++ Builder workaround: we should be able to define a compile time
   // constant and pass that as a single template parameter to ct_imp<T,bool>,
   // however compiler bugs prevent this - instead pass three bool's to
   // ct_imp<T,bool,bool,bool> and add an extra partial specialisation
   // of ct_imp to handle the logic. (JM)
   typedef typename multi_index::detail::ct_imp<
      T,
      std::is_pointer<T>::value,
      std::is_arithmetic<T>::value,
      std::is_enum<T>::value
   >::param_type param_type;
};

template <typename T>
struct call_traits<T&>
{
   typedef T& value_type;
   typedef T& reference;
   typedef const T& const_reference;
   typedef T& param_type;  // hh removed const
};

}  /* multi_index */
