/* Copyright 2003-2014 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

#include <brigand/sequences/list.hpp>
#include <multi_index/detail/allocator_utilities.hpp>
#include <multi_index/detail/copy_map.hpp>
#include <multi_index/detail/do_not_copy_elements_tag.hpp>
#include <multi_index/detail/node_type.hpp>
#include <multi_index/detail/tuple_support.hpp>
#include <multi_index/detail/vartempl_support.hpp>
#include <multi_index_container_fwd.hpp>
#include <multi_index/tuple.hpp>
#include <utility>

namespace multi_index{

namespace detail{

/* The role of this class is threefold:
 *   - tops the linear hierarchy of indices.
 *   - terminates some cascading backbone function calls (insert_, etc.),
 *   - grants access to the backbone functions of the final
 *     multi_index_container class (for access restriction reasons, these
 *     cannot be called directly from the index classes.)
 */

struct lvalue_tag{};
struct rvalue_tag{};
struct emplaced_tag{};

template<typename Value,typename IndexSpecifierList,typename Allocator>
class index_base
{
protected:
  typedef index_node_base<Value,Allocator>    node_type;
  typedef typename multi_index_node_type<
    Value,
    IndexSpecifierList,
    Allocator
  >::type                                     final_node_type;
  typedef multi_index_container<
    Value,IndexSpecifierList,Allocator>       final_type;
  typedef tuples::null_type                   ctor_args_list;
  typedef typename
  multi_index::detail::allocator::rebind_to<
    Allocator,
    typename Allocator::value_type
  >::type                                     final_allocator_type;
  typedef brigand::list<>                     index_type_list;
  typedef brigand::list<>                     iterator_type_list;
  typedef brigand::list<>                     const_iterator_type_list;
  typedef copy_map<
    final_node_type,
    final_allocator_type>                     copy_map_type;

private:
  typedef Value                               value_type;

protected:
  explicit index_base(const ctor_args_list&,const Allocator&){}

  index_base(
    const index_base<Value,IndexSpecifierList,Allocator>&,
    do_not_copy_elements_tag)
  {}

  void copy_(
    const index_base<Value,IndexSpecifierList,Allocator>&,const copy_map_type&)
  {}

  final_node_type* insert_(const value_type& v,final_node_type*& x,lvalue_tag)
  {
    x=final().allocate_node();
    try {
      multi_index::detail::allocator::construct(&x->value(),v);
    }
    catch(...){
      final().deallocate_node(x);
      throw;
    }
    return x;
  }

  final_node_type* insert_(const value_type& v,final_node_type*& x,rvalue_tag)
  {
    x=final().allocate_node();
    try {
      /* This shoud have used a modified, T&&-compatible version of
       * detail::allocator::construct, but
       *  <multi_index/detail/allocator_utilities.hpp> is too old and venerable to
       * mess with; besides, it is a general internal utility and the imperfect
       * perfect forwarding emulation of Boost.Move might break other libs.
       */

      new (&x->value()) value_type(std::move(const_cast<value_type&>(v)));
    }
    catch(...){
      final().deallocate_node(x);
      throw;
    }
    return x;
  }

  final_node_type* insert_(const value_type&,final_node_type*& x,emplaced_tag)
  {
    return x;
  }

  final_node_type* insert_(
    const value_type& v,node_type*,final_node_type*& x,lvalue_tag)
  {
    return insert_(v,x,lvalue_tag());
  }

  final_node_type* insert_(
    const value_type& v,node_type*,final_node_type*& x,rvalue_tag)
  {
    return insert_(v,x,rvalue_tag());
  }

  final_node_type* insert_(
    const value_type&,node_type*,final_node_type*& x,emplaced_tag)
  {
    return x;
  }

  void erase_(node_type* x)
  {
    multi_index::detail::allocator::destroy(&x->value());
  }

  void delete_node_(node_type* x)
  {
    multi_index::detail::allocator::destroy(&x->value());
  }

  void clear_(){}

  void swap_(index_base<Value,IndexSpecifierList,Allocator>&){}

  void swap_elements_(index_base<Value,IndexSpecifierList,Allocator>&){}

  bool replace_(const value_type& v,node_type* x,lvalue_tag)
  {
    x->value()=v;
    return true;
  }

  bool replace_(const value_type& v,node_type* x,rvalue_tag)
  {
    x->value()=std::move(const_cast<value_type&>(v));
    return true;
  }

  bool modify_(node_type*){return true;}

  bool modify_rollback_(node_type*){return true;}

  /* access to backbone memfuns of Final class */

  final_type&       final(){return *static_cast<final_type*>(this);}
  const final_type& final()const{return *static_cast<const final_type*>(this);}

  final_node_type* final_header()const{return final().header();}

  bool        final_empty_()const{return final().empty_();}
  std::size_t final_size_()const{return final().size_();}
  std::size_t final_max_size_()const{return final().max_size_();}

  std::pair<final_node_type*,bool> final_insert_(const value_type& x)
    {return final().insert_(x);}
  std::pair<final_node_type*,bool> final_insert_rv_(const value_type& x)
    {return final().insert_rv_(x);}
  template<typename T>
  std::pair<final_node_type*,bool> final_insert_ref_(const T& t)
    {return final().insert_ref_(t);}
  template<typename T>
  std::pair<final_node_type*,bool> final_insert_ref_(T& t)
    {return final().insert_ref_(t);}

  template<typename... Args>
  std::pair<final_node_type*,bool> final_emplace_(
    Args&&... args)
  {
    return final().emplace_(std::forward<Args>(args)...);
  }

  std::pair<final_node_type*,bool> final_insert_(
    const value_type& x,final_node_type* position)
    {return final().insert_(x,position);}
  std::pair<final_node_type*,bool> final_insert_rv_(
    const value_type& x,final_node_type* position)
    {return final().insert_rv_(x,position);}
  template<typename T>
  std::pair<final_node_type*,bool> final_insert_ref_(
    const T& t,final_node_type* position)
    {return final().insert_ref_(t,position);}
  template<typename T>
  std::pair<final_node_type*,bool> final_insert_ref_(
    T& t,final_node_type* position)
    {return final().insert_ref_(t,position);}

  template<typename... Args>
  std::pair<final_node_type*,bool> final_emplace_hint_(
    final_node_type* position,Args&&... args)
  {
    return final().emplace_hint_(position,std::forward<Args>(args)...);
  }

  void final_erase_(final_node_type* x){final().erase_(x);}

  void final_delete_node_(final_node_type* x){final().delete_node_(x);}
  void final_delete_all_nodes_(){final().delete_all_nodes_();}
  void final_clear_(){final().clear_();}

  void final_swap_(final_type& x){final().swap_(x);}

  bool final_replace_(
    const value_type& k,final_node_type* x)
    {return final().replace_(k,x);}
  bool final_replace_rv_(
    const value_type& k,final_node_type* x)
    {return final().replace_rv_(k,x);}

  template<typename Modifier>
  bool final_modify_(Modifier& mod,final_node_type* x)
    {return final().modify_(mod,x);}

  template<typename Modifier,typename Rollback>
  bool final_modify_(Modifier& mod,Rollback& back,final_node_type* x)
    {return final().modify_(mod,back,x);}
};

} /* namespace multi_index::detail */

} /* namespace multi_index */
