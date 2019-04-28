/* Copyright 2003-2015 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

#include <multi_index/detail/auto_space.hpp>
#include <multi_index/detail/hash_index_node.hpp>
#include <algorithm>
#include <cstddef>
#include <limits.h>

namespace multi_index{

namespace detail{

/* bucket structure for use by hashed indices */

// MACROS
// ------

#define IS_ULONG32 ((((ULONG_MAX>>16)>>16)>>16)>>15)==0

// CONSTANTS
// ---------

#if IS_ULONG32                            // IS_ULONG32

#define MULTI_INDEX_BA_SIZES {53ul, 97ul, 193ul, 389ul, 769ul, 1543ul, 3079ul, 6151ul, 12289ul, 24593ul, 49157ul, 98317ul, 196613ul, 393241ul, 786433ul, 1572869ul, 3145739ul, 6291469ul, 12582917ul, 25165843ul, 50331653ul, 100663319ul, 201326611ul, 402653189ul, 805306457ul, 1610612741ul, 3221225473ul, 4294967291ul}
#define MULTI_INDEX_BA_LENGTH 28

#else                                     // IS_ULONG64

  /* obtained with aid from
   *   http://javaboutique.internet.com/prime_numb/
   *   http://www.rsok.com/~jrm/next_ten_primes.html
   * and verified with
   *   http://www.alpertron.com.ar/ECM.HTM
   */

#define MULTI_INDEX_BA_SIZES {53ul, 97ul, 193ul, 389ul, 769ul, 1543ul, 3079ul, 6151ul, 12289ul, 24593ul, 49157ul, 98317ul, 196613ul, 393241ul, 786433ul, 1572869ul, 3145739ul, 6291469ul, 12582917ul, 25165843ul, 50331653ul, 100663319ul, 201326611ul, 402653189ul, 805306457ul, 1610612741ul, 3221225473ul, 6442450939ul, 12884901893ul, 25769803751ul, 51539607551ul, 103079215111ul, 206158430209ul, 412316860441ul, 824633720831ul, 1649267441651ul, 3298534883309ul, 6597069766657ul, 13194139533299ul, 26388279066623ul, 52776558133303ul, 105553116266489ul, 211106232532969ul, 422212465066001ul, 844424930131963ul, 1688849860263953ul, 3377699720527861ul, 6755399441055731ul, 13510798882111483ul, 27021597764222939ul, 54043195528445957ul, 108086391056891903ul, 216172782113783843ul, 432345564227567621ul, 864691128455135207ul, 1729382256910270481ul, 3458764513820540933ul, 6917529027641081903ul, 13835058055282163729ul, 18446744073709551557ul}
#define MULTI_INDEX_BA_LENGTH 60

#endif                                    // IS_ULONG32

template<bool _=true> /* templatized to have in-header static var defs */
class bucket_array_base
{
protected:
  constexpr bucket_array_base() = default;
  ~bucket_array_base() = default;
  bucket_array_base( const bucket_array_base& ) = delete;
  bucket_array_base& operator=( const bucket_array_base& ) = delete;

  static const std::size_t sizes[MULTI_INDEX_BA_LENGTH];

  static std::size_t size_index(std::size_t n)
  {
    const std::size_t *bound=std::lower_bound(sizes,sizes+sizes_length,n);
    if(bound==sizes+sizes_length)--bound;
    return bound-sizes;
  }

  static std::size_t position(std::size_t hash,std::size_t size_index_)
  {
    /* Accelerate hash%sizes[size_index_] by replacing with a switch on
     * hash%Ci expressions, each Ci a compile-time constant, which the
     * compiler can implement without using integer division.
     *
     * Replaced to remove to Boost preprocessor, these are manually
     * expanded.
     */

    switch(size_index_){
      default: /* never used */
      case 0:     return hash % 53ul;
      case 1:     return hash % 97ul;
      case 2:     return hash % 193ul;
      case 3:     return hash % 389ul;
      case 4:     return hash % 769ul;
      case 5:     return hash % 1543ul;
      case 6:     return hash % 3079ul;
      case 7:     return hash % 6151ul;
      case 8:     return hash % 12289ul;
      case 9:     return hash % 24593ul;
      case 10:    return hash % 49157ul;
      case 11:    return hash % 98317ul;
      case 12:    return hash % 196613ul;
      case 13:    return hash % 393241ul;
      case 14:    return hash % 786433ul;
      case 15:    return hash % 1572869ul;
      case 16:    return hash % 3145739ul;
      case 17:    return hash % 6291469ul;
      case 18:    return hash % 12582917ul;
      case 19:    return hash % 25165843ul;
      case 20:    return hash % 50331653ul;
      case 21:    return hash % 100663319ul;
      case 22:    return hash % 201326611ul;
      case 23:    return hash % 402653189ul;
      case 24:    return hash % 805306457ul;
      case 25:    return hash % 1610612741ul;
      case 26:    return hash % 3221225473ul;
#if IS_ULONG32                                // IS_ULONG32
      case 27:    return hash % 4294967291ul;
#else                                         // IS_ULONG64
      case 27:    return hash % 6442450939ul;
      case 28:    return hash % 12884901893ul;
      case 29:    return hash % 25769803751ul;
      case 30:    return hash % 51539607551ul;
      case 31:    return hash % 103079215111ul;
      case 32:    return hash % 206158430209ul;
      case 33:    return hash % 412316860441ul;
      case 34:    return hash % 824633720831ul;
      case 35:    return hash % 1649267441651ul;
      case 36:    return hash % 3298534883309ul;
      case 37:    return hash % 6597069766657ul;
      case 38:    return hash % 13194139533299ul;
      case 39:    return hash % 26388279066623ul;
      case 40:    return hash % 52776558133303ul;
      case 41:    return hash % 105553116266489ul;
      case 42:    return hash % 211106232532969ul;
      case 43:    return hash % 422212465066001ul;
      case 44:    return hash % 844424930131963ul;
      case 45:    return hash % 1688849860263953ul;
      case 46:    return hash % 3377699720527861ul;
      case 47:    return hash % 6755399441055731ul;
      case 48:    return hash % 13510798882111483ul;
      case 49:    return hash % 27021597764222939ul;
      case 50:    return hash % 54043195528445957ul;
      case 51:    return hash % 108086391056891903ul;
      case 52:    return hash % 216172782113783843ul;
      case 53:    return hash % 432345564227567621ul;
      case 54:    return hash % 864691128455135207ul;
      case 55:    return hash % 1729382256910270481ul;
      case 56:    return hash % 3458764513820540933ul;
      case 57:    return hash % 6917529027641081903ul;
      case 58:    return hash % 13835058055282163729ul;
      case 59:    return hash % 18446744073709551557ul;
#endif                                        // IS_ULONG32
    }
  }

private:
  static const std::size_t sizes_length;
};

template<bool _>
const std::size_t bucket_array_base<_>::sizes[] = MULTI_INDEX_BA_SIZES;

template<bool _>
const std::size_t bucket_array_base<_>::sizes_length = MULTI_INDEX_BA_LENGTH;


template<typename Allocator>
class bucket_array:bucket_array_base<>
{
  typedef bucket_array_base<>                        super;
  typedef hashed_index_base_node_impl<
    typename multi_index::detail::allocator::rebind_to<
      Allocator,
      char
    >::type
  >                                                  base_node_impl_type;

public:
  typedef typename base_node_impl_type::base_pointer base_pointer;
  typedef typename base_node_impl_type::pointer      pointer;

  bucket_array(const Allocator& al,pointer end_,std::size_t size_):
    size_index_(super::size_index(size_)),
    spc(al,super::sizes[size_index_]+1)
  {
    clear(end_);
  }

  std::size_t size()const
  {
    return super::sizes[size_index_];
  }

  std::size_t position(std::size_t hash)const
  {
    return super::position(hash,size_index_);
  }

  base_pointer begin()const{return buckets();}
  base_pointer end()const{return buckets()+size();}
  base_pointer at(std::size_t n)const{return buckets()+n;}

  void clear(pointer end_)
  {
    for(base_pointer x=begin(),y=end();x!=y;++x)x->prior()=pointer(0);
    end()->prior()=end_->prior()=end_;
    end_->next()=end();
 }

  void swap(bucket_array& x)
  {
    std::swap(size_index_,x.size_index_);
    spc.swap(x.spc);
  }

private:
  std::size_t                               size_index_;
  auto_space<base_node_impl_type,Allocator> spc;

  base_pointer buckets()const
  {
    return spc.data();
  }
};

template<typename Allocator>
void swap(bucket_array<Allocator>& x,bucket_array<Allocator>& y)
{
  x.swap(y);
}

// CLEANUP
// -------

#undef MULTI_INDEX_BA_SIZES
#undef MULTI_INDEX_BA_LENGTH

} /* namespace multi_index::detail */

} /* namespace multi_index */
