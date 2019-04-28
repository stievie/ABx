/* Copyright 2003-2013 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See Boost website at http://www.boost.org/
 */

#pragma once

#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>

namespace multi_index{
namespace detail{

/* Allocator adaption layer. Some stdlibs provide allocators without rebind
 * and template ctors. These facilities are simulated with the external
 * template class rebind_to and the aid of partial_std_allocator_wrapper.
 */

namespace allocator{

/* partial_std_allocator_wrapper inherits the functionality of a std
 * allocator while providing a templatized ctor and other bits missing
 * in some stdlib implementation or another.
 */

template<typename Type>
class partial_std_allocator_wrapper:public std::allocator<Type>
{
public:
  /* Oddly enough, STLport does not define std::allocator<void>::value_type
   * when configured to work without partial template specialization.
   * No harm in supplying the definition here unconditionally.
   */

  typedef Type value_type;

  partial_std_allocator_wrapper(){};

  template<typename Other>
  partial_std_allocator_wrapper(const partial_std_allocator_wrapper<Other>&){}

  partial_std_allocator_wrapper(const std::allocator<Type>& x):
    std::allocator<Type>(x)
  {
  };

};

template<typename Allocator>
struct is_partial_std_allocator
{
  static const bool value = false;
};

/* rebind operations for defective std allocators */

template<typename Allocator,typename Type>
struct partial_std_allocator_rebind_to
{
  typedef partial_std_allocator_wrapper<Type> type;
};

/* rebind operation in all other cases */

template<typename Allocator>
struct rebinder
{
  template<typename Type>
  struct result
  {
      typedef typename std::allocator_traits<Allocator>::template rebind_alloc<Type> other;
  };
};

template<typename Allocator,typename Type>
struct compliant_allocator_rebind_to
{
  typedef typename rebinder<Allocator>::template result<Type>::other type;
};

/* rebind front-end */

template<typename Allocator,typename Type>
struct rebind_to:
  std::conditional<
    is_partial_std_allocator<Allocator>::value,
    partial_std_allocator_rebind_to<Allocator,Type>,
    compliant_allocator_rebind_to<Allocator,Type>
  >::type
{
};

/* allocator-independent versions of construct and destroy */

template<typename Type>
void construct(void* p,const Type& t)
{
  new (p) Type(t);
}

template<typename Type>
void destroy(const Type* p)
{
  p->~Type();

}

} /* namespace multi_index::detail::allocator */

} /* namespace multi_index::detail */
} /* namespace multi_index */
