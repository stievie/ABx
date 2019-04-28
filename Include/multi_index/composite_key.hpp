/* Copyright 2003-2015 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

#include <brigand/functions/eval_if.hpp>
#include <brigand/functions/arithmetic/identity.hpp>
#include <brigand/functions/logical/or.hpp>
#include <multi_index/detail/cons_stdtuple.hpp>
#include <multi_index/detail/tuple_support.hpp>
#include <multi_index/config.hpp>
#include <functional>
#include <type_traits>


/* A composite key stores n key extractors and "computes" the
 * result on a given value as a packed reference to the value and
 * the composite key itself. Actual invocations to the component
 * key extractors are lazily performed when executing an operation
 * on composite_key results (equality, comparison, hashing.)
 * As the other key extractors in Boost.MultiIndex, composite_key<T,...>
 * is  overloaded to work on chained pointers to T and reference_wrappers
 * of T.
 */

/* This user_definable macro limits the number of elements of a composite
 * key; useful for shortening resulting symbol names (MSVC++ 6.0, for
 * instance has problems coping with very long symbol names.)
 * NB: This cannot exceed the maximum number of arguments of
 * tuple. In Boost 1.32, the limit is 10.
 */

#ifndef MULTI_INDEX_COMPOSITE_KEY_SIZE
# define MULTI_INDEX_COMPOSITE_KEY_SIZE 10
#endif

namespace multi_index{

namespace detail{

/* n-th key extractor of a composite key */

template<typename CompositeKey,int N>
struct nth_key_from_value
{
  typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
  typedef typename brigand::eval_if_c<
    N<tuples::length<key_extractor_tuple>::value,
    tuples::element<N,key_extractor_tuple>,
    brigand::identity<tuples::null_type>
  >::type                                            type;
};

/* nth_composite_key_##name<CompositeKey,N>::type yields
 * functor<nth_key_from_value<CompositeKey,N> >, or tuples::null_type
 * if N exceeds the length of the composite key.
 */

#define MULTI_INDEX_CK_NTH_COMPOSITE_KEY_FUNCTOR(name,functor)                \
template<typename KeyFromValue>                                               \
struct key_##name                                                             \
{                                                                             \
  typedef functor<typename KeyFromValue::result_type> type;                   \
};                                                                            \
                                                                              \
template<>                                                                    \
struct key_##name <tuples::null_type>                                         \
{                                                                             \
  typedef tuples::null_type type;                                             \
};                                                                            \
                                                                              \
template<typename CompositeKey,int  N>                                        \
struct nth_composite_key_##name                                               \
{                                                                             \
  typedef typename nth_key_from_value<CompositeKey,N>::type key_from_value;   \
  typedef typename key_##name <key_from_value>::type type;                    \
};

/* nth_composite_key_equal_to
 * nth_composite_key_less
 * nth_composite_key_greater
 * nth_composite_key_hash
 */

MULTI_INDEX_CK_NTH_COMPOSITE_KEY_FUNCTOR(equal_to,MULTI_INDEX_EQUAL_TO)
MULTI_INDEX_CK_NTH_COMPOSITE_KEY_FUNCTOR(less,MULTI_INDEX_LESS)
MULTI_INDEX_CK_NTH_COMPOSITE_KEY_FUNCTOR(greater,MULTI_INDEX_GREATER)
MULTI_INDEX_CK_NTH_COMPOSITE_KEY_FUNCTOR(hash,MULTI_INDEX_HASH)

/* used for defining equality and comparison ops of composite_key_result */

struct generic_operator_equal
{
  template<typename T,typename Q>
  bool operator()(const T& x,const Q& y)const{return x==y;}
};

using generic_operator_equal_tuple = tuple_builder<
  MULTI_INDEX_COMPOSITE_KEY_SIZE,
  generic_operator_equal
>;

struct generic_operator_less
{
  template<typename T,typename Q>
  bool operator()(const T& x,const Q& y)const{return x<y;}
};

using generic_operator_less_tuple = tuple_builder<
  MULTI_INDEX_COMPOSITE_KEY_SIZE,
  generic_operator_less
>;

/* Metaprogramming machinery for implementing equality, comparison and
 * hashing operations of composite_key_result.
 *
 * equal_* checks for equality between composite_key_results and
 * between those and tuples, accepting a tuple of basic equality functors.
 * compare_* does lexicographical comparison.
 * hash_* computes a combination of elementwise hash values.
 */

template
<
  typename KeyCons1,typename Value1,
  typename KeyCons2, typename Value2,
  typename EqualCons
>
struct equal_ckey_ckey; /* fwd decl. */

template
<
  typename KeyCons1,typename Value1,
  typename KeyCons2, typename Value2,
  typename EqualCons
>
struct equal_ckey_ckey_terminal
{
  static bool compare(
    const KeyCons1&,const Value1&,
    const KeyCons2&,const Value2&,
    const EqualCons&)
  {
    return true;
  }
};

template
<
  typename KeyCons1,typename Value1,
  typename KeyCons2, typename Value2,
  typename EqualCons
>
struct equal_ckey_ckey_normal
{
  static bool compare(
    const KeyCons1& c0,const Value1& v0,
    const KeyCons2& c1,const Value2& v1,
    const EqualCons& eq)
  {
    if(!eq.get_head()(c0.get_head()(v0),c1.get_head()(v1)))return false;
    return equal_ckey_ckey<
      typename KeyCons1::tail_type,Value1,
      typename KeyCons2::tail_type,Value2,
      typename EqualCons::tail_type
    >::compare(c0.get_tail(),v0,c1.get_tail(),v1,eq.get_tail());
  }
};

template
<
  typename KeyCons1,typename Value1,
  typename KeyCons2, typename Value2,
  typename EqualCons
>
struct equal_ckey_ckey:
  std::conditional<
    brigand::or_<
      std::is_same<KeyCons1,tuples::null_type>,
      std::is_same<KeyCons2,tuples::null_type>
    >::value,
    equal_ckey_ckey_terminal<KeyCons1,Value1,KeyCons2,Value2,EqualCons>,
    equal_ckey_ckey_normal<KeyCons1,Value1,KeyCons2,Value2,EqualCons>
  >::type
{
};

template
<
  typename KeyCons,typename Value,
  typename ValCons,typename EqualCons
>
struct equal_ckey_cval; /* fwd decl. */

template
<
  typename KeyCons,typename Value,
  typename ValCons,typename EqualCons
>
struct equal_ckey_cval_terminal
{
  static bool compare(
    const KeyCons&,const Value&,const ValCons&,const EqualCons&)
  {
    return true;
  }

  static bool compare(
    const ValCons&,const KeyCons&,const Value&,const EqualCons&)
  {
    return true;
  }
};

template
<
  typename KeyCons,typename Value,
  typename ValCons,typename EqualCons
>
struct equal_ckey_cval_normal
{
  static bool compare(
    const KeyCons& c,const Value& v,const ValCons& vc,
    const EqualCons& eq)
  {
    if(!eq.get_head()(c.get_head()(v),vc.get_head()))return false;
    return equal_ckey_cval<
      typename KeyCons::tail_type,Value,
      typename ValCons::tail_type,
      typename EqualCons::tail_type
    >::compare(c.get_tail(),v,vc.get_tail(),eq.get_tail());
  }

  static bool compare(
    const ValCons& vc,const KeyCons& c,const Value& v,
    const EqualCons& eq)
  {
    if(!eq.get_head()(vc.get_head(),c.get_head()(v)))return false;
    return equal_ckey_cval<
      typename KeyCons::tail_type,Value,
      typename ValCons::tail_type,
      typename EqualCons::tail_type
    >::compare(vc.get_tail(),c.get_tail(),v,eq.get_tail());
  }
};

template
<
  typename KeyCons,typename Value,
  typename ValCons,typename EqualCons
>
struct equal_ckey_cval:
  std::conditional<
    brigand::or_<
      std::is_same<KeyCons,tuples::null_type>,
      std::is_same<ValCons,tuples::null_type>
    >::value,
    equal_ckey_cval_terminal<KeyCons,Value,ValCons,EqualCons>,
    equal_ckey_cval_normal<KeyCons,Value,ValCons,EqualCons>
  >::type
{
};

template
<
  typename KeyCons1,typename Value1,
  typename KeyCons2, typename Value2,
  typename CompareCons
>
struct compare_ckey_ckey; /* fwd decl. */

template
<
  typename KeyCons1,typename Value1,
  typename KeyCons2, typename Value2,
  typename CompareCons
>
struct compare_ckey_ckey_terminal
{
  static bool compare(
    const KeyCons1&,const Value1&,
    const KeyCons2&,const Value2&,
    const CompareCons&)
  {
    return false;
  }
};

template
<
  typename KeyCons1,typename Value1,
  typename KeyCons2, typename Value2,
  typename CompareCons
>
struct compare_ckey_ckey_normal
{
  static bool compare(
    const KeyCons1& c0,const Value1& v0,
    const KeyCons2& c1,const Value2& v1,
    const CompareCons& comp)
  {
    if(comp.get_head()(c0.get_head()(v0),c1.get_head()(v1)))return true;
    if(comp.get_head()(c1.get_head()(v1),c0.get_head()(v0)))return false;
    return compare_ckey_ckey<
      typename KeyCons1::tail_type,Value1,
      typename KeyCons2::tail_type,Value2,
      typename CompareCons::tail_type
    >::compare(c0.get_tail(),v0,c1.get_tail(),v1,comp.get_tail());
  }
};

template
<
  typename KeyCons1,typename Value1,
  typename KeyCons2, typename Value2,
  typename CompareCons
>
struct compare_ckey_ckey:
  std::conditional<
    brigand::or_<
      std::is_same<KeyCons1,tuples::null_type>,
      std::is_same<KeyCons2,tuples::null_type>
    >::value,
    compare_ckey_ckey_terminal<KeyCons1,Value1,KeyCons2,Value2,CompareCons>,
    compare_ckey_ckey_normal<KeyCons1,Value1,KeyCons2,Value2,CompareCons>
  >::type
{
};

template
<
  typename KeyCons,typename Value,
  typename ValCons,typename CompareCons
>
struct compare_ckey_cval; /* fwd decl. */

template
<
  typename KeyCons,typename Value,
  typename ValCons,typename CompareCons
>
struct compare_ckey_cval_terminal
{
  static bool compare(
    const KeyCons&,const Value&,const ValCons&,const CompareCons&)
  {
    return false;
  }

  static bool compare(
    const ValCons&,const KeyCons&,const Value&,const CompareCons&)
  {
    return false;
  }
};

template
<
  typename KeyCons,typename Value,
  typename ValCons,typename CompareCons
>
struct compare_ckey_cval_normal
{
  static bool compare(
    const KeyCons& c,const Value& v,const ValCons& vc,
    const CompareCons& comp)
  {
    if(comp.get_head()(c.get_head()(v),vc.get_head()))return true;
    if(comp.get_head()(vc.get_head(),c.get_head()(v)))return false;
    return compare_ckey_cval<
      typename KeyCons::tail_type,Value,
      typename ValCons::tail_type,
      typename CompareCons::tail_type
    >::compare(c.get_tail(),v,vc.get_tail(),comp.get_tail());
  }

  static bool compare(
    const ValCons& vc,const KeyCons& c,const Value& v,
    const CompareCons& comp)
  {
    if(comp.get_head()(vc.get_head(),c.get_head()(v)))return true;
    if(comp.get_head()(c.get_head()(v),vc.get_head()))return false;
    return compare_ckey_cval<
      typename KeyCons::tail_type,Value,
      typename ValCons::tail_type,
      typename CompareCons::tail_type
    >::compare(vc.get_tail(),c.get_tail(),v,comp.get_tail());
  }
};

template
<
  typename KeyCons,typename Value,
  typename ValCons,typename CompareCons
>
struct compare_ckey_cval:
  std::conditional<
    brigand::or_<
      std::is_same<KeyCons,tuples::null_type>,
      std::is_same<ValCons,tuples::null_type>
    >::value,
    compare_ckey_cval_terminal<KeyCons,Value,ValCons,CompareCons>,
    compare_ckey_cval_normal<KeyCons,Value,ValCons,CompareCons>
  >::type
{
};

template<typename KeyCons,typename Value,typename HashCons>
struct hash_ckey; /* fwd decl. */

template<typename KeyCons,typename Value,typename HashCons>
struct hash_ckey_terminal
{
  static std::size_t hash(
    const KeyCons&,const Value&,const HashCons&,std::size_t carry)
  {
    return carry;
  }
};

template<typename KeyCons,typename Value,typename HashCons>
struct hash_ckey_normal
{
  static std::size_t hash(
    const KeyCons& c,const Value& v,const HashCons& h,std::size_t carry=0)
  {
    /* same hashing formula as hash_combine */

    carry^=h.get_head()(c.get_head()(v))+0x9e3779b9+(carry<<6)+(carry>>2);
    return hash_ckey<
      typename KeyCons::tail_type,Value,
      typename HashCons::tail_type
    >::hash(c.get_tail(),v,h.get_tail(),carry);
  }
};

template<typename KeyCons,typename Value,typename HashCons>
struct hash_ckey:
  std::conditional<
    std::is_same<KeyCons,tuples::null_type>::value,
    hash_ckey_terminal<KeyCons,Value,HashCons>,
    hash_ckey_normal<KeyCons,Value,HashCons>
  >::type
{
};

template<typename ValCons,typename HashCons>
struct hash_cval; /* fwd decl. */

template<typename ValCons,typename HashCons>
struct hash_cval_terminal
{
  static std::size_t hash(const ValCons&,const HashCons&,std::size_t carry)
  {
    return carry;
  }
};

template<typename ValCons,typename HashCons>
struct hash_cval_normal
{
  static std::size_t hash(
    const ValCons& vc,const HashCons& h,std::size_t carry=0)
  {
    carry^=h.get_head()(vc.get_head())+0x9e3779b9+(carry<<6)+(carry>>2);
    return hash_cval<
      typename ValCons::tail_type,
      typename HashCons::tail_type
    >::hash(vc.get_tail(),h.get_tail(),carry);
  }
};

template<typename ValCons,typename HashCons>
struct hash_cval:
  std::conditional<
    std::is_same<ValCons,tuples::null_type>::value,
    hash_cval_terminal<ValCons,HashCons>,
    hash_cval_normal<ValCons,HashCons>
  >::type
{
};

} /* namespace multi_index::detail */

/* composite_key_result */

template<typename CompositeKey>
struct composite_key_result
{
  typedef CompositeKey                            composite_key_type;
  typedef typename composite_key_type::value_type value_type;

  composite_key_result(
    const composite_key_type& composite_key_,const value_type& value_):
    composite_key(composite_key_),value(value_)
  {}

  const composite_key_type& composite_key;
  const value_type&         value;
};

/* composite_key */

template<typename Value, typename... Us>
struct composite_key:
  private tuple<Us...>
{
private:
  typedef tuple<Us...> super;

public:
  typedef super                               key_extractor_tuple;
  typedef Value                               value_type;
  typedef composite_key_result<composite_key> result_type;

  template <typename... Ts>
  composite_key(Ts&&... ts):
    super(std::forward<Ts>(ts)...)
  {}

  composite_key(const key_extractor_tuple& x):super(x){}

  const key_extractor_tuple& key_extractors()const{return *this;}
  key_extractor_tuple&       key_extractors(){return *this;}

  template<typename ChainedPtr>

  typename std::enable_if<
    !std::is_convertible<const ChainedPtr&,const value_type&>::value,result_type>::type

  operator()(const ChainedPtr& x)const
  {
    return operator()(*x);
  }

  result_type operator()(const value_type& x)const
  {
    return result_type(*this,x);
  }

  result_type operator()(const std::reference_wrapper<const value_type>& x)const
  {
    return result_type(*this,x.get());
  }

  result_type operator()(const std::reference_wrapper<value_type>& x)const
  {
    return result_type(*this,x.get());
  }
};

/* comparison operators */

/* == */

template<typename CompositeKey1,typename CompositeKey2>
inline bool operator==(
  const composite_key_result<CompositeKey1>& x,
  const composite_key_result<CompositeKey2>& y)
{
  typedef typename CompositeKey1::key_extractor_tuple key_extractor_tuple1;
  typedef typename CompositeKey1::value_type          value_type1;
  typedef typename CompositeKey2::key_extractor_tuple key_extractor_tuple2;
  typedef typename CompositeKey2::value_type          value_type2;

  static_assert(
    tuples::length<key_extractor_tuple1>::value==
    tuples::length<key_extractor_tuple2>::value, "");

  return detail::equal_ckey_ckey<
    key_extractor_tuple1,value_type1,
    key_extractor_tuple2,value_type2,
    detail::generic_operator_equal_tuple
  >::compare(
    x.composite_key.key_extractors(),x.value,
    y.composite_key.key_extractors(),y.value,
    detail::generic_operator_equal_tuple());
}


template <typename CompositeKey, typename... Us>
inline bool operator==(
  const composite_key_result<CompositeKey>& x,
  const tuple<Us...>& y)
{
  typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
  typedef typename CompositeKey::value_type              value_type;
  typedef tuple<Us...> key_tuple;

  static_assert(
    tuples::length<key_extractor_tuple>::value==
    tuples::length<key_tuple>::value, "");

  return detail::equal_ckey_cval<
    key_extractor_tuple,value_type,
    key_tuple,detail::generic_operator_equal_tuple
  >::compare(
    x.composite_key.key_extractors(),x.value,
    y,detail::generic_operator_equal_tuple());
}

template <typename... Us, typename CompositeKey>
inline bool operator==(
  const tuple<Us...>& x,
  const composite_key_result<CompositeKey>& y)
{
  typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
  typedef typename CompositeKey::value_type              value_type;
  typedef tuple<Us...> key_tuple;

  static_assert(
    tuples::length<key_extractor_tuple>::value==
    tuples::length<key_tuple>::value, "");

  return detail::equal_ckey_cval<
    key_extractor_tuple,value_type,
    key_tuple,detail::generic_operator_equal_tuple
  >::compare(
    x,y.composite_key.key_extractors(),
    y.value,detail::generic_operator_equal_tuple());
}

template<typename CompositeKey,typename... Values>
inline bool operator==(
  const composite_key_result<CompositeKey>& x,
  const std::tuple<Values...>& y)
{
  typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
  typedef typename CompositeKey::value_type          value_type;
  typedef std::tuple<Values...>                      key_tuple;
  typedef typename detail::cons_stdtuple_ctor<
    key_tuple>::result_type                          cons_key_tuple;

  static_assert(
    static_cast<std::size_t>(tuples::length<key_extractor_tuple>::value)==
    std::tuple_size<key_tuple>::value, "");

  return detail::equal_ckey_cval<
    key_extractor_tuple,value_type,
    cons_key_tuple,detail::generic_operator_equal_tuple
  >::compare(
    x.composite_key.key_extractors(),x.value,
    detail::make_cons_stdtuple(y),detail::generic_operator_equal_tuple());
}

template<typename CompositeKey,typename... Values>
inline bool operator==(
  const std::tuple<Values...>& x,
  const composite_key_result<CompositeKey>& y)
{
  typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
  typedef typename CompositeKey::value_type          value_type;
  typedef std::tuple<Values...>                      key_tuple;
  typedef typename detail::cons_stdtuple_ctor<
    key_tuple>::result_type                          cons_key_tuple;

  static_assert(
    static_cast<std::size_t>(tuples::length<key_extractor_tuple>::value)==
    std::tuple_size<key_tuple>::value, "");

  return detail::equal_ckey_cval<
    key_extractor_tuple,value_type,
    cons_key_tuple,detail::generic_operator_equal_tuple
  >::compare(
    detail::make_cons_stdtuple(x),y.composite_key.key_extractors(),
    y.value,detail::generic_operator_equal_tuple());
}

/* < */

template<typename CompositeKey1,typename CompositeKey2>
inline bool operator<(
  const composite_key_result<CompositeKey1>& x,
  const composite_key_result<CompositeKey2>& y)
{
  typedef typename CompositeKey1::key_extractor_tuple key_extractor_tuple1;
  typedef typename CompositeKey1::value_type          value_type1;
  typedef typename CompositeKey2::key_extractor_tuple key_extractor_tuple2;
  typedef typename CompositeKey2::value_type          value_type2;

  return detail::compare_ckey_ckey<
   key_extractor_tuple1,value_type1,
   key_extractor_tuple2,value_type2,
   detail::generic_operator_less_tuple
  >::compare(
    x.composite_key.key_extractors(),x.value,
    y.composite_key.key_extractors(),y.value,
    detail::generic_operator_less_tuple());
}

template <typename CompositeKey, typename... Us>
inline bool operator<(
  const composite_key_result<CompositeKey>& x,
  const tuple<Us...>& y)
{
  typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
  typedef typename CompositeKey::value_type              value_type;
  typedef tuple<Us...>                                   key_tuple;

  return detail::compare_ckey_cval<
    key_extractor_tuple,value_type,
    key_tuple,detail::generic_operator_less_tuple
  >::compare(
    x.composite_key.key_extractors(),x.value,
    y,detail::generic_operator_less_tuple());
}

template <typename... Us, typename CompositeKey>
inline bool operator<(
  const tuple<Us...>& x,
  const composite_key_result<CompositeKey>& y)
{
  typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
  typedef typename CompositeKey::value_type              value_type;
  typedef tuple<Us...>                                   key_tuple;

  return detail::compare_ckey_cval<
    key_extractor_tuple,value_type,
    key_tuple,detail::generic_operator_less_tuple
  >::compare(
    x,y.composite_key.key_extractors(),
    y.value,detail::generic_operator_less_tuple());
}

template<typename CompositeKey,typename... Values>
inline bool operator<(
  const composite_key_result<CompositeKey>& x,
  const std::tuple<Values...>& y)
{
  typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
  typedef typename CompositeKey::value_type          value_type;
  typedef std::tuple<Values...>                      key_tuple;
  typedef typename detail::cons_stdtuple_ctor<
    key_tuple>::result_type                          cons_key_tuple;

  return detail::compare_ckey_cval<
    key_extractor_tuple,value_type,
    cons_key_tuple,detail::generic_operator_less_tuple
  >::compare(
    x.composite_key.key_extractors(),x.value,
    detail::make_cons_stdtuple(y),detail::generic_operator_less_tuple());
}

template<typename CompositeKey,typename... Values>
inline bool operator<(
  const std::tuple<Values...>& x,
  const composite_key_result<CompositeKey>& y)
{
  typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
  typedef typename CompositeKey::value_type          value_type;
  typedef std::tuple<Values...>                      key_tuple;
  typedef typename detail::cons_stdtuple_ctor<
    key_tuple>::result_type                          cons_key_tuple;

  return detail::compare_ckey_cval<
    key_extractor_tuple,value_type,
    cons_key_tuple,detail::generic_operator_less_tuple
  >::compare(
    detail::make_cons_stdtuple(x),y.composite_key.key_extractors(),
    y.value,detail::generic_operator_less_tuple());
}

/* rest of comparison operators */

#define MULTI_INDEX_CK_COMPLETE_COMP_OPS(t1,t2,a1,a2)                        \
template<t1,t2> inline bool operator!=(const a1& x,const a2& y)              \
{                                                                            \
  return !(x==y);                                                            \
}                                                                            \
                                                                             \
template<t1,t2> inline bool operator>(const a1& x,const a2& y)               \
{                                                                            \
  return y<x;                                                                \
}                                                                            \
                                                                             \
template<t1,t2> inline bool operator>=(const a1& x,const a2& y)              \
{                                                                            \
  return !(x<y);                                                             \
}                                                                            \
                                                                             \
template<t1,t2> inline bool operator<=(const a1& x,const a2& y)              \
{                                                                            \
  return !(y<x);                                                             \
}

MULTI_INDEX_CK_COMPLETE_COMP_OPS(
  typename CompositeKey1,
  typename CompositeKey2,
  composite_key_result<CompositeKey1>,
  composite_key_result<CompositeKey2>
)

MULTI_INDEX_CK_COMPLETE_COMP_OPS(
  typename CompositeKey,
  typename... Us,
  composite_key_result<CompositeKey>,
  tuple<Us...>
)

MULTI_INDEX_CK_COMPLETE_COMP_OPS(
  typename... Us,
  typename CompositeKey,
  tuple<Us...>,
  composite_key_result<CompositeKey>
)

MULTI_INDEX_CK_COMPLETE_COMP_OPS(
  typename CompositeKey,
  typename... Values,
  composite_key_result<CompositeKey>,
  std::tuple<Values...>
)

MULTI_INDEX_CK_COMPLETE_COMP_OPS(
  typename CompositeKey,
  typename... Values,
  std::tuple<Values...>,
  composite_key_result<CompositeKey>
)

/* composite_key_equal_to */

template <typename... Us>
struct composite_key_equal_to:
  private tuple<Us...>
{
private:
  typedef tuple<Us...> super;

public:
  typedef super key_eq_tuple;

  template <typename... Ts>
  composite_key_equal_to(Ts&&... ts):
    super(std::forward<Ts>(ts)...)
  {}

  const key_eq_tuple& key_eqs()const{return *this;}
  key_eq_tuple&       key_eqs(){return *this;}

  template<typename CompositeKey1,typename CompositeKey2>
  bool operator()(
    const composite_key_result<CompositeKey1> & x,
    const composite_key_result<CompositeKey2> & y)const
  {
    typedef typename CompositeKey1::key_extractor_tuple key_extractor_tuple1;
    typedef typename CompositeKey1::value_type          value_type1;
    typedef typename CompositeKey2::key_extractor_tuple key_extractor_tuple2;
    typedef typename CompositeKey2::value_type          value_type2;

    static_assert(
      tuples::length<key_extractor_tuple1>::value<=
      tuples::length<key_eq_tuple>::value&&
      tuples::length<key_extractor_tuple1>::value==
      tuples::length<key_extractor_tuple2>::value, "");

    return detail::equal_ckey_ckey<
      key_extractor_tuple1,value_type1,
      key_extractor_tuple2,value_type2,
      key_eq_tuple
    >::compare(
      x.composite_key.key_extractors(),x.value,
      y.composite_key.key_extractors(),y.value,
      key_eqs());
  }

  template <typename CompositeKey, typename... Ts>
  bool operator()(
    const composite_key_result<CompositeKey>& x,
    const tuple<Ts...>& y)const
  {
    typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
    typedef typename CompositeKey::value_type              value_type;
    typedef tuple<Ts...>                                   key_tuple;

    static_assert(
      tuples::length<key_extractor_tuple>::value<=
      tuples::length<key_eq_tuple>::value&&
      tuples::length<key_extractor_tuple>::value==
      tuples::length<key_tuple>::value, "");

    return detail::equal_ckey_cval<
      key_extractor_tuple,value_type,
      key_tuple,key_eq_tuple
    >::compare(x.composite_key.key_extractors(),x.value,y,key_eqs());
  }

  template <typename... Ts, typename CompositeKey>
  bool operator()(
    const tuple<Ts...>& x,
    const composite_key_result<CompositeKey>& y)const
  {
    typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
    typedef typename CompositeKey::value_type              value_type;
    typedef tuple<Ts...>                                   key_tuple;

    static_assert(
      tuples::length<key_tuple>::value<=
      tuples::length<key_eq_tuple>::value&&
      tuples::length<key_tuple>::value==
      tuples::length<key_extractor_tuple>::value, "");

    return detail::equal_ckey_cval<
      key_extractor_tuple,value_type,
      key_tuple,key_eq_tuple
    >::compare(x,y.composite_key.key_extractors(),y.value,key_eqs());
  }

  template<typename CompositeKey,typename... Values>
  bool operator()(
    const composite_key_result<CompositeKey>& x,
    const std::tuple<Values...>& y)const
  {
    typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
    typedef typename CompositeKey::value_type          value_type;
    typedef std::tuple<Values...>                      key_tuple;
    typedef typename detail::cons_stdtuple_ctor<
      key_tuple>::result_type                          cons_key_tuple;

    static_assert(
      tuples::length<key_extractor_tuple>::value<=
      tuples::length<key_eq_tuple>::value&&
      static_cast<std::size_t>(tuples::length<key_extractor_tuple>::value)==
      std::tuple_size<key_tuple>::value, "");

    return detail::equal_ckey_cval<
      key_extractor_tuple,value_type,
      cons_key_tuple,key_eq_tuple
    >::compare(
      x.composite_key.key_extractors(),x.value,
      detail::make_cons_stdtuple(y),key_eqs());
  }

  template<typename CompositeKey,typename... Values>
  bool operator()(
    const std::tuple<Values...>& x,
    const composite_key_result<CompositeKey>& y)const
  {
    typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
    typedef typename CompositeKey::value_type          value_type;
    typedef std::tuple<Values...>                      key_tuple;
    typedef typename detail::cons_stdtuple_ctor<
      key_tuple>::result_type                          cons_key_tuple;

    static_assert(
      std::tuple_size<key_tuple>::value<=
      static_cast<std::size_t>(tuples::length<key_eq_tuple>::value)&&
      std::tuple_size<key_tuple>::value==
      static_cast<std::size_t>(tuples::length<key_extractor_tuple>::value), "");

    return detail::equal_ckey_cval<
      key_extractor_tuple,value_type,
      cons_key_tuple,key_eq_tuple
    >::compare(
      detail::make_cons_stdtuple(x),y.composite_key.key_extractors(),
      y.value,key_eqs());
  }
};

/* composite_key_compare */

template <typename... Us>
struct composite_key_compare: private tuple<Us...>
{
private:
  typedef tuple<Us...> super;

public:
  typedef super key_comp_tuple;

  template <typename... Ts>
  composite_key_compare(Ts&&... ts):
    super(std::forward<Ts>(ts)...)
  {}

  const key_comp_tuple& key_comps()const{return *this;}
  key_comp_tuple&       key_comps(){return *this;}

  template<typename CompositeKey1,typename CompositeKey2>
  bool operator()(
    const composite_key_result<CompositeKey1> & x,
    const composite_key_result<CompositeKey2> & y)const
  {
    typedef typename CompositeKey1::key_extractor_tuple key_extractor_tuple1;
    typedef typename CompositeKey1::value_type          value_type1;
    typedef typename CompositeKey2::key_extractor_tuple key_extractor_tuple2;
    typedef typename CompositeKey2::value_type          value_type2;

    static_assert(
      tuples::length<key_extractor_tuple1>::value<=
      tuples::length<key_comp_tuple>::value||
      tuples::length<key_extractor_tuple2>::value<=
      tuples::length<key_comp_tuple>::value, "");

    return detail::compare_ckey_ckey<
      key_extractor_tuple1,value_type1,
      key_extractor_tuple2,value_type2,
      key_comp_tuple
    >::compare(
      x.composite_key.key_extractors(),x.value,
      y.composite_key.key_extractors(),y.value,
      key_comps());
  }

  template<typename CompositeKey,typename Value>
  bool operator()(
    const composite_key_result<CompositeKey>& x,
    const Value& y)const
  {
    return operator()(x,multi_index::make_tuple(std::cref(y)));
  }

  template <typename CompositeKey, typename... Ts>
  bool operator()(
    const composite_key_result<CompositeKey>& x,
    const tuple<Ts...>& y)const
  {
    typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
    typedef typename CompositeKey::value_type              value_type;
    typedef tuple<Ts...> key_tuple;

    static_assert(
      tuples::length<key_extractor_tuple>::value<=
      tuples::length<key_comp_tuple>::value||
      tuples::length<key_tuple>::value<=
      tuples::length<key_comp_tuple>::value, "");

    return detail::compare_ckey_cval<
      key_extractor_tuple,value_type,
      key_tuple,key_comp_tuple
    >::compare(x.composite_key.key_extractors(),x.value,y,key_comps());
  }

  template<typename Value,typename CompositeKey>
  bool operator()(
    const Value& x,
    const composite_key_result<CompositeKey>& y)const
  {
    return operator()(multi_index::make_tuple(std::cref(x)),y);
  }

  template <typename... Ts, typename CompositeKey>
  bool operator()(
    const tuple<Ts...>& x,
    const composite_key_result<CompositeKey>& y)const
  {
    typedef typename CompositeKey::key_extractor_tuple     key_extractor_tuple;
    typedef typename CompositeKey::value_type              value_type;
    typedef tuple<Ts...> key_tuple;

    static_assert(
      tuples::length<key_tuple>::value<=
      tuples::length<key_comp_tuple>::value||
      tuples::length<key_extractor_tuple>::value<=
      tuples::length<key_comp_tuple>::value, "");

    return detail::compare_ckey_cval<
      key_extractor_tuple,value_type,
      key_tuple,key_comp_tuple
    >::compare(x,y.composite_key.key_extractors(),y.value,key_comps());
  }

  template<typename CompositeKey,typename... Values>
  bool operator()(
    const composite_key_result<CompositeKey>& x,
    const std::tuple<Values...>& y)const
  {
    typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
    typedef typename CompositeKey::value_type          value_type;
    typedef std::tuple<Values...>                      key_tuple;
    typedef typename detail::cons_stdtuple_ctor<
      key_tuple>::result_type                          cons_key_tuple;

    static_assert(
      tuples::length<key_extractor_tuple>::value<=
      tuples::length<key_comp_tuple>::value||
      std::tuple_size<key_tuple>::value<=
      static_cast<std::size_t>(tuples::length<key_comp_tuple>::value), "");

    return detail::compare_ckey_cval<
      key_extractor_tuple,value_type,
      cons_key_tuple,key_comp_tuple
    >::compare(
      x.composite_key.key_extractors(),x.value,
      detail::make_cons_stdtuple(y),key_comps());
  }

  template<typename CompositeKey,typename... Values>
  bool operator()(
    const std::tuple<Values...>& x,
    const composite_key_result<CompositeKey>& y)const
  {
    typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
    typedef typename CompositeKey::value_type          value_type;
    typedef std::tuple<Values...>                      key_tuple;
    typedef typename detail::cons_stdtuple_ctor<
      key_tuple>::result_type                          cons_key_tuple;

    static_assert(
      std::tuple_size<key_tuple>::value<=
      static_cast<std::size_t>(tuples::length<key_comp_tuple>::value)||
      tuples::length<key_extractor_tuple>::value<=
      tuples::length<key_comp_tuple>::value, "");

    return detail::compare_ckey_cval<
      key_extractor_tuple,value_type,
      cons_key_tuple,key_comp_tuple
    >::compare(
      detail::make_cons_stdtuple(x),y.composite_key.key_extractors(),
      y.value,key_comps());
  }
};

/* composite_key_hash */

template <typename... Us>
struct composite_key_hash: private tuple<Us...>
{
private:
  typedef tuple<Us...> super;

public:
  typedef super key_hasher_tuple;

  template <typename... Ts>
  composite_key_hash(Ts&&... ts):
    super(std::forward<Ts>(ts)...)
  {}

  composite_key_hash(const key_hasher_tuple& x):super(x){}

  const key_hasher_tuple& key_hash_functions()const{return *this;}
  key_hasher_tuple&       key_hash_functions(){return *this;}

  template<typename CompositeKey>
  std::size_t operator()(const composite_key_result<CompositeKey> & x)const
  {
    typedef typename CompositeKey::key_extractor_tuple key_extractor_tuple;
    typedef typename CompositeKey::value_type          value_type;

    static_assert(
      tuples::length<key_extractor_tuple>::value==
      tuples::length<key_hasher_tuple>::value, "");

    return detail::hash_ckey<
      key_extractor_tuple,value_type,
      key_hasher_tuple
    >::hash(x.composite_key.key_extractors(),x.value,key_hash_functions());
  }

  template <typename... Ts>
  std::size_t operator()(const tuple<Ts...>& x) const
  {
    typedef tuple<Ts...> key_tuple;

    static_assert(
      tuples::length<key_tuple>::value==
      tuples::length<key_hasher_tuple>::value, "");

    return detail::hash_cval<
      key_tuple,key_hasher_tuple
    >::hash(x,key_hash_functions());
  }

  template<typename... Values>
  std::size_t operator()(const std::tuple<Values...>& x)const
  {
    typedef std::tuple<Values...>                key_tuple;
    typedef typename detail::cons_stdtuple_ctor<
      key_tuple>::result_type                    cons_key_tuple;

    static_assert(
      std::tuple_size<key_tuple>::value==
      static_cast<std::size_t>(tuples::length<key_hasher_tuple>::value), "");

    return detail::hash_cval<
      cons_key_tuple,key_hasher_tuple
    >::hash(detail::make_cons_stdtuple(x),key_hash_functions());
  }
};

/* Instantiations of the former functors with "natural" basic components:
 * composite_key_result_equal_to uses std::equal_to of the values.
 * composite_key_result_less     uses std::less.
 * composite_key_result_greater  uses std::greater.
 * composite_key_result_hash     uses hash.
 */

template <typename CompositeKeyResult, template <typename, int> class Pred>
using key_result_builder = indexed_tuple_builder<
  MULTI_INDEX_COMPOSITE_KEY_SIZE,
  Pred,
  typename CompositeKeyResult::composite_key_type
>;


template <typename CompositeKeyResult, template <typename, int> class Pred>
using composite_key_extractor = extract_tuple_element<
  key_result_builder<CompositeKeyResult, Pred>
>;

template <template <typename...> class Super, typename CompositeKeyResult, template <typename, int> class Pred>
using composite_key_super = tuple_as_class<
  Super,
  composite_key_extractor<CompositeKeyResult, Pred>
>;

template <typename CompositeKeyResult>
using ck_equal_to_super = composite_key_super<
  composite_key_equal_to,
  CompositeKeyResult,
  detail::nth_composite_key_equal_to
>;


template <typename CompositeKeyResult>
struct composite_key_result_equal_to: private ck_equal_to_super<CompositeKeyResult>
{
private:
  typedef ck_equal_to_super<CompositeKeyResult> super;

public:
  typedef CompositeKeyResult  first_argument_type;
  typedef first_argument_type second_argument_type;
  typedef bool                result_type;

  using super::operator();
};


template <typename CompositeKeyResult>
using ck_less_super = composite_key_super<
  composite_key_compare,
  CompositeKeyResult,
  detail::nth_composite_key_less
>;


template<typename CompositeKeyResult>
struct composite_key_result_less: private ck_less_super<CompositeKeyResult>
{
private:
  typedef ck_less_super<CompositeKeyResult> super;

public:
  typedef CompositeKeyResult  first_argument_type;
  typedef first_argument_type second_argument_type;
  typedef bool                result_type;

  using super::operator();
};


template <typename CompositeKeyResult>
using ck_greater_super = composite_key_super<
  composite_key_compare,
  CompositeKeyResult,
  detail::nth_composite_key_greater
>;


template<typename CompositeKeyResult>
struct composite_key_result_greater: private ck_greater_super<CompositeKeyResult>
{
private:
  typedef ck_greater_super<CompositeKeyResult> super;

public:
  typedef CompositeKeyResult  first_argument_type;
  typedef first_argument_type second_argument_type;
  typedef bool                result_type;

  using super::operator();
};


template <typename CompositeKeyResult>
using ck_hash_super = composite_key_super<
  composite_key_hash,
  CompositeKeyResult,
  detail::nth_composite_key_hash
>;


template<typename CompositeKeyResult>
struct composite_key_result_hash: private ck_hash_super<CompositeKeyResult>
{
private:
  typedef ck_hash_super<CompositeKeyResult> super;

public:
  typedef CompositeKeyResult argument_type;
  typedef std::size_t        result_type;

  using super::operator();
};

} /* namespace multi_index */

/* Specializations of std::equal_to, std::less, std::greater and hash
 * for composite_key_results enabling interoperation with tuples of values.
 */

namespace std{

template<typename CompositeKey>
struct equal_to<multi_index::composite_key_result<CompositeKey> >:
  multi_index::composite_key_result_equal_to<
    multi_index::composite_key_result<CompositeKey>
  >
{};

template<typename CompositeKey>
struct less<multi_index::composite_key_result<CompositeKey> >:
  multi_index::composite_key_result_less<
    multi_index::composite_key_result<CompositeKey>
  >
{};

template<typename CompositeKey>
struct greater<multi_index::composite_key_result<CompositeKey> >:
  multi_index::composite_key_result_greater<
    multi_index::composite_key_result<CompositeKey>
  >
{};

template<typename CompositeKey>
struct hash<multi_index::composite_key_result<CompositeKey> >:
  multi_index::composite_key_result_hash<
    multi_index::composite_key_result<CompositeKey>
  >
{};

} /* namespace std */

#undef MULTI_INDEX_CK_COMPLETE_COMP_OPS
#undef MULTI_INDEX_CK_NTH_COMPOSITE_KEY_FUNCTOR
#undef MULTI_INDEX_COMPOSITE_KEY_SIZE
