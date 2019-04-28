/* Copyright 2003-2015 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 *
 * The internal implementation of red-black trees is based on that of SGI STL
 * stl_tree.h file:
 *
 * Copyright (c) 1996,1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

#pragma once

#include <brigand/functions/if.hpp>
#include <brigand/sequences/front.hpp>
#include <algorithm>
#include <multi_index/detail/call_traits.hpp>
#include <multi_index/detail/bidir_node_iterator.hpp>
#include <multi_index/detail/do_not_copy_elements_tag.hpp>
#include <multi_index/detail/index_node_base.hpp>
#include <multi_index/detail/modify_key_adaptor.hpp>
#include <multi_index/detail/ord_index_node.hpp>
#include <multi_index/detail/ord_index_ops.hpp>
#include <multi_index/detail/scope_guard.hpp>
#include <multi_index/detail/unbounded.hpp>
#include <multi_index/detail/value_compare.hpp>
#include <multi_index/detail/vartempl_support.hpp>
#include <multi_index/detail/ord_index_impl_fwd.hpp>
#include <multi_index/tuple.hpp>
#include <initializer_list>
#include <iterator>
#include <utility>
#include <type_traits>

namespace multi_index{

namespace detail{

/* ordered_index adds a layer of ordered indexing to a given Super and accepts
 * an augmenting policy for optional addition of order statistics.
 */

/* Most of the implementation of unique and non-unique indices is
 * shared. We tell from one another on instantiation time by using
 * these tags.
 */

struct ordered_unique_tag{};
struct ordered_non_unique_tag{};

template<
  typename KeyFromValue,typename Compare,
  typename SuperMeta,typename TagList,typename Category,typename AugmentPolicy
>
class ordered_index;

template<
  typename KeyFromValue,typename Compare,
  typename SuperMeta,typename TagList,typename Category,typename AugmentPolicy
>
class ordered_index_impl:
  protected SuperMeta::type

{
  typedef typename SuperMeta::type                   super;

protected:
  typedef ordered_index_node<
    AugmentPolicy,typename super::node_type>         node_type;

protected: /* for the benefit of AugmentPolicy::augmented_interface */
  typedef typename node_type::impl_type              node_impl_type;
  typedef typename node_impl_type::pointer           node_impl_pointer;

public:
  /* types */

  typedef typename KeyFromValue::result_type         key_type;
  typedef typename node_type::value_type             value_type;
  typedef KeyFromValue                               key_from_value;
  typedef Compare                                    key_compare;
  typedef value_comparison<
    value_type,KeyFromValue,Compare>                 value_compare;
  typedef tuple<key_from_value,key_compare>          ctor_args;
  typedef typename super::final_allocator_type       allocator_type;
  typedef typename std::allocator_traits<
    allocator_type>::value_type&                     reference;
  typedef typename std::allocator_traits<
    allocator_type>::value_type const&               const_reference;

  typedef bidir_node_iterator<node_type>             iterator;
  typedef iterator                                   const_iterator;

  typedef std::size_t                                size_type;
  typedef std::ptrdiff_t                             difference_type;
  typedef typename std::allocator_traits<
    allocator_type>::pointer                        pointer;
  typedef typename std::allocator_traits<
    allocator_type>::const_pointer                  const_pointer;
  typedef typename
    std::reverse_iterator<iterator>                reverse_iterator;
  typedef typename
    std::reverse_iterator<const_iterator>          const_reverse_iterator;
  typedef TagList                                    tag_list;

protected:
  typedef typename super::final_node_type            final_node_type;
  typedef tuples::cons<
    ctor_args,
    typename super::ctor_args_list>                  ctor_args_list;
  typedef brigand::push_front<
    typename super::index_type_list,
    ordered_index<
      KeyFromValue,Compare,
      SuperMeta,TagList,Category,AugmentPolicy
    > >                                              index_type_list;
  typedef brigand::push_front<
    typename super::iterator_type_list,
    iterator>                                        iterator_type_list;
  typedef brigand::push_front<
    typename super::const_iterator_type_list,
    const_iterator>                                  const_iterator_type_list;
  typedef typename super::copy_map_type              copy_map_type;

protected:
  typedef typename call_traits<
    value_type>::param_type                          value_param_type;
  typedef typename call_traits<
    key_type>::param_type                            key_param_type;

  /* Needed to avoid commas in MULTI_INDEX_OVERLOADS_TO_VARTEMPL
   * expansion.
   */

  typedef std::pair<iterator,bool>                   emplace_return_type;

public:

  /* construct/copy/destroy
   * Default and copy ctors are in the protected section as indices are
   * not supposed to be created on their own. No range ctor either.
   * Assignment operators defined at ordered_index rather than here.
   */

  allocator_type get_allocator()const noexcept
  {
    return this->final().get_allocator();
  }

  /* iterators */

  iterator
    begin()noexcept{return make_iterator(leftmost());}
  const_iterator
    begin()const noexcept{return make_iterator(leftmost());}
  iterator
    end()noexcept{return make_iterator(header());}
  const_iterator
    end()const noexcept{return make_iterator(header());}
  reverse_iterator
    rbegin()noexcept{return reverse_iterator(end());}
  const_reverse_iterator
    rbegin()const noexcept{return const_reverse_iterator(end());}
  reverse_iterator
    rend()noexcept{return reverse_iterator(begin());}
  const_reverse_iterator
    rend()const noexcept{return const_reverse_iterator(begin());}
  const_iterator
    cbegin()const noexcept{return begin();}
  const_iterator
    cend()const noexcept{return end();}
  const_reverse_iterator
    crbegin()const noexcept{return rbegin();}
  const_reverse_iterator
    crend()const noexcept{return rend();}

  iterator iterator_to(const value_type& x)
  {
    return make_iterator(node_from_value<node_type>(&x));
  }

  const_iterator iterator_to(const value_type& x)const
  {
    return make_iterator(node_from_value<node_type>(&x));
  }

  /* capacity */

  bool      empty()const noexcept{return this->final_empty_();}
  size_type size()const noexcept{return this->final_size_();}
  size_type max_size()const noexcept{return this->final_max_size_();}

  /* modifiers */

  MULTI_INDEX_OVERLOADS_TO_VARTEMPL(
    emplace_return_type,emplace,emplace_impl)

  MULTI_INDEX_OVERLOADS_TO_VARTEMPL_EXTRA_ARG(
    iterator,emplace_hint,emplace_hint_impl,iterator,position)

  std::pair<iterator,bool> insert(const value_type& x)
  {
    std::pair<final_node_type*,bool> p=this->final_insert_(x);
    return std::pair<iterator,bool>(make_iterator(p.first),p.second);
  }

  std::pair<iterator,bool> insert(value_type&& x)
  {
    std::pair<final_node_type*,bool> p=this->final_insert_rv_(x);
    return std::pair<iterator,bool>(make_iterator(p.first),p.second);
  }

  iterator insert(iterator position,const value_type& x)
  {
    std::pair<final_node_type*,bool> p=this->final_insert_(
      x,static_cast<final_node_type*>(position.get_node()));
    return make_iterator(p.first);
  }

  iterator insert(iterator position,value_type&& x)
  {
    std::pair<final_node_type*,bool> p=this->final_insert_rv_(
      x,static_cast<final_node_type*>(position.get_node()));
    return make_iterator(p.first);
  }

  template<typename InputIterator>
  void insert(InputIterator first,InputIterator last)
  {
    node_type* hint=header(); /* end() */
    for(;first!=last;++first){
      hint=this->final_insert_ref_(
        *first,static_cast<final_node_type*>(hint)).first;
      node_type::increment(hint);
    }
  }

  void insert(std::initializer_list<value_type> list)
  {
    insert(list.begin(),list.end());
  }

  iterator erase(iterator position)
  {
    this->final_erase_(static_cast<final_node_type*>(position++.get_node()));
    return position;
  }

  size_type erase(key_param_type x)
  {
    std::pair<iterator,iterator> p=equal_range(x);
    size_type s=0;
    while(p.first!=p.second){
      p.first=erase(p.first);
      ++s;
    }
    return s;
  }

  iterator erase(iterator first,iterator last)
  {
    while(first!=last){
      first=erase(first);
    }
    return first;
  }

  bool replace(iterator position,const value_type& x)
  {
    return this->final_replace_(
      x,static_cast<final_node_type*>(position.get_node()));
  }

  bool replace(iterator position,value_type&& x)
  {
    return this->final_replace_rv_(
      x,static_cast<final_node_type*>(position.get_node()));
  }

  template<typename Modifier>
  bool modify(iterator position,Modifier mod)
  {

    return this->final_modify_(
      mod,static_cast<final_node_type*>(position.get_node()));
  }

  template<typename Modifier,typename Rollback>
  bool modify(iterator position,Modifier mod,Rollback back_)
  {

    return this->final_modify_(
      mod,back_,static_cast<final_node_type*>(position.get_node()));
  }

  template<typename Modifier>
  bool modify_key(iterator position,Modifier mod)
  {
    return modify(
      position,modify_key_adaptor<Modifier,value_type,KeyFromValue>(mod,key));
  }

  template<typename Modifier,typename Rollback>
  bool modify_key(iterator position,Modifier mod,Rollback back_)
  {
    return modify(
      position,
      modify_key_adaptor<Modifier,value_type,KeyFromValue>(mod,key),
      modify_key_adaptor<Rollback,value_type,KeyFromValue>(back_,key));
  }

  void swap(
    ordered_index<
      KeyFromValue,Compare,SuperMeta,TagList,Category,AugmentPolicy>& x)
  {
    this->final_swap_(x.final());
  }

  void clear()noexcept
  {
    this->final_clear_();
  }

  /* observers */

  key_from_value key_extractor()const{return key;}
  key_compare    key_comp()const{return comp_;}
  value_compare  value_comp()const{return value_compare(key,comp_);}

  /* set operations */

  /* Internally, these ops rely on const_iterator being the same
   * type as iterator.
   */

  template<typename CompatibleKey>
  iterator find(const CompatibleKey& x)const
  {
    return make_iterator(ordered_index_find(root(),header(),key,x,comp_));
  }

  template<typename CompatibleKey,typename CompatibleCompare>
  iterator find(
    const CompatibleKey& x,const CompatibleCompare& comp)const
  {
    return make_iterator(ordered_index_find(root(),header(),key,x,comp));
  }

  template<typename CompatibleKey>
  size_type count(const CompatibleKey& x)const
  {
    return count(x,comp_);
  }

  template<typename CompatibleKey,typename CompatibleCompare>
  size_type count(const CompatibleKey& x,const CompatibleCompare& comp)const
  {
    std::pair<iterator,iterator> p=equal_range(x,comp);
    size_type n=std::distance(p.first,p.second);
    return n;
  }

  template<typename CompatibleKey>
  iterator lower_bound(const CompatibleKey& x)const
  {
    return make_iterator(
      ordered_index_lower_bound(root(),header(),key,x,comp_));
  }

  template<typename CompatibleKey,typename CompatibleCompare>
  iterator lower_bound(
    const CompatibleKey& x,const CompatibleCompare& comp)const
  {
    return make_iterator(
      ordered_index_lower_bound(root(),header(),key,x,comp));
  }

  template<typename CompatibleKey>
  iterator upper_bound(const CompatibleKey& x)const
  {
    return make_iterator(
      ordered_index_upper_bound(root(),header(),key,x,comp_));
  }

  template<typename CompatibleKey,typename CompatibleCompare>
  iterator upper_bound(
    const CompatibleKey& x,const CompatibleCompare& comp)const
  {
    return make_iterator(
      ordered_index_upper_bound(root(),header(),key,x,comp));
  }

  template<typename CompatibleKey>
  std::pair<iterator,iterator> equal_range(
    const CompatibleKey& x)const
  {
    std::pair<node_type*,node_type*> p=
      ordered_index_equal_range(root(),header(),key,x,comp_);
    return std::pair<iterator,iterator>(
      make_iterator(p.first),make_iterator(p.second));
  }

  template<typename CompatibleKey,typename CompatibleCompare>
  std::pair<iterator,iterator> equal_range(
    const CompatibleKey& x,const CompatibleCompare& comp)const
  {
    std::pair<node_type*,node_type*> p=
      ordered_index_equal_range(root(),header(),key,x,comp);
    return std::pair<iterator,iterator>(
      make_iterator(p.first),make_iterator(p.second));
  }

  /* range */

  template<typename LowerBounder,typename UpperBounder>
  std::pair<iterator,iterator>
  range(LowerBounder lower,UpperBounder upper)const
  {
    typedef typename brigand::if_<
      std::is_same<LowerBounder,unbounded_type>,
      typename brigand::if_<
        std::is_same<UpperBounder,unbounded_type>,
        both_unbounded_tag,
        lower_unbounded_tag
      >::type,
      typename brigand::if_<
        std::is_same<UpperBounder,unbounded_type>,
        upper_unbounded_tag,
        none_unbounded_tag
      >::type
    >::type dispatch;

    return range(lower,upper,dispatch());
  }

protected:
  ordered_index_impl(const ctor_args_list& args_list,const allocator_type& al):
    super(args_list.get_tail(),al),
    key(tuples::get<0>(args_list.get_head())),
    comp_(tuples::get<1>(args_list.get_head()))
  {
    empty_initialize();
  }

  ordered_index_impl(
    const ordered_index_impl<
      KeyFromValue,Compare,SuperMeta,TagList,Category,AugmentPolicy>& x):
    super(x),

    key(x.key),
    comp_(x.comp_)
  {
    /* Copy ctor just takes the key and compare objects from x. The rest is
     * done in a subsequent call to copy_().
     */
  }

  ordered_index_impl(
     const ordered_index_impl<
       KeyFromValue,Compare,SuperMeta,TagList,Category,AugmentPolicy>& x,
     do_not_copy_elements_tag):
    super(x,do_not_copy_elements_tag()),

    key(x.key),
    comp_(x.comp_)
  {
    empty_initialize();
  }

  ~ordered_index_impl()
  {
    /* the container is guaranteed to be empty by now */
  }

  iterator       make_iterator(node_type* node){return iterator(node);}
  const_iterator make_iterator(node_type* node)const
                   {return const_iterator(node);}

  void copy_(
    const ordered_index_impl<
      KeyFromValue,Compare,SuperMeta,TagList,Category,AugmentPolicy>& x,
    const copy_map_type& map)
  {
    if(!x.root()){
      empty_initialize();
    }
    else{
      header()->color()=x.header()->color();
      AugmentPolicy::copy(x.header()->impl(),header()->impl());

      node_type* root_cpy=map.find(static_cast<final_node_type*>(x.root()));
      header()->parent()=root_cpy->impl();

      node_type* leftmost_cpy=map.find(
        static_cast<final_node_type*>(x.leftmost()));
      header()->left()=leftmost_cpy->impl();

      node_type* rightmost_cpy=map.find(
        static_cast<final_node_type*>(x.rightmost()));
      header()->right()=rightmost_cpy->impl();

      typedef typename copy_map_type::const_iterator copy_map_iterator;
      for(copy_map_iterator it=map.begin(),it_end=map.end();it!=it_end;++it){
        node_type* org=it->first;
        node_type* cpy=it->second;

        cpy->color()=org->color();
        AugmentPolicy::copy(org->impl(),cpy->impl());

        node_impl_pointer parent_org=org->parent();
        if(parent_org==node_impl_pointer(0))cpy->parent()=node_impl_pointer(0);
        else{
          node_type* parent_cpy=map.find(
            static_cast<final_node_type*>(node_type::from_impl(parent_org)));
          cpy->parent()=parent_cpy->impl();
          if(parent_org->left()==org->impl()){
            parent_cpy->left()=cpy->impl();
          }
          else if(parent_org->right()==org->impl()){
            /* header() does not satisfy this nor the previous check */
            parent_cpy->right()=cpy->impl();
          }
        }

        if(org->left()==node_impl_pointer(0))
          cpy->left()=node_impl_pointer(0);
        if(org->right()==node_impl_pointer(0))
          cpy->right()=node_impl_pointer(0);
      }
    }

    super::copy_(x,map);
  }

  template<typename Variant>
  final_node_type* insert_(
    value_param_type v,final_node_type*& x,Variant variant)
  {
    link_info inf;
    if(!link_point(key(v),inf,Category())){
      return static_cast<final_node_type*>(node_type::from_impl(inf.pos));
    }

    final_node_type* res=super::insert_(v,x,variant);
    if(res==x){
      node_impl_type::link(
        static_cast<node_type*>(x)->impl(),inf.side,inf.pos,header()->impl());
    }
    return res;
  }

  template<typename Variant>
  final_node_type* insert_(
    value_param_type v,node_type* position,final_node_type*& x,Variant variant)
  {
    link_info inf;
    if(!hinted_link_point(key(v),position,inf,Category())){
      return static_cast<final_node_type*>(node_type::from_impl(inf.pos));
    }

    final_node_type* res=super::insert_(v,position,x,variant);
    if(res==x){
      node_impl_type::link(
        static_cast<node_type*>(x)->impl(),inf.side,inf.pos,header()->impl());
    }
    return res;
  }

  void erase_(node_type* x)
  {
    node_impl_type::rebalance_for_erase(
      x->impl(),header()->parent(),header()->left(),header()->right());
    super::erase_(x);
  }

  void delete_all_nodes_()
  {
    delete_all_nodes(root());
  }

  void clear_()
  {
    super::clear_();
    empty_initialize();
  }

  void swap_(
    ordered_index_impl<
      KeyFromValue,Compare,SuperMeta,TagList,Category,AugmentPolicy>& x)
  {
    std::swap(key,x.key);
    std::swap(comp_,x.comp_);
    super::swap_(x);
  }

  void swap_elements_(
    ordered_index_impl<
      KeyFromValue,Compare,SuperMeta,TagList,Category,AugmentPolicy>& x)
  {
    super::swap_elements_(x);
  }

  template<typename Variant>
  bool replace_(value_param_type v,node_type* x,Variant variant)
  {
    if(in_place(v,x,Category())){
      return super::replace_(v,x,variant);
    }

    node_type* next=x;
    node_type::increment(next);

    node_impl_type::rebalance_for_erase(
      x->impl(),header()->parent(),header()->left(),header()->right());

    try {
      link_info inf;
      if(link_point(key(v),inf,Category())&&super::replace_(v,x,variant)){
        node_impl_type::link(x->impl(),inf.side,inf.pos,header()->impl());
        return true;
      }
      node_impl_type::restore(x->impl(),next->impl(),header()->impl());
      return false;
    }
    catch(...){
      node_impl_type::restore(x->impl(),next->impl(),header()->impl());
      throw;
    }
  }

  bool modify_(node_type* x)
  {
    bool b;
    try {
      b=in_place(x->value(),x,Category());
    }
    catch(...){
      erase_(x);
      throw;
    }
    if(!b){
      node_impl_type::rebalance_for_erase(
        x->impl(),header()->parent(),header()->left(),header()->right());
      try {
        link_info inf;
        if(!link_point(key(x->value()),inf,Category())){
          super::erase_(x);
          return false;
        }
        node_impl_type::link(x->impl(),inf.side,inf.pos,header()->impl());
      }
      catch(...){
        super::erase_(x);
        throw;
      }
    }

    try {
      if(!super::modify_(x)){
        node_impl_type::rebalance_for_erase(
          x->impl(),header()->parent(),header()->left(),header()->right());
        return false;
      }
      else return true;
    }
    catch(...){
      node_impl_type::rebalance_for_erase(
        x->impl(),header()->parent(),header()->left(),header()->right());
      throw;
    }
  }

  bool modify_rollback_(node_type* x)
  {
    if(in_place(x->value(),x,Category())){
      return super::modify_rollback_(x);
    }

    node_type* next=x;
    node_type::increment(next);

    node_impl_type::rebalance_for_erase(
      x->impl(),header()->parent(),header()->left(),header()->right());

    try {
      link_info inf;
      if(link_point(key(x->value()),inf,Category())&&
         super::modify_rollback_(x)){
        node_impl_type::link(x->impl(),inf.side,inf.pos,header()->impl());
        return true;
      }
      node_impl_type::restore(x->impl(),next->impl(),header()->impl());
      return false;
    }
    catch(...){
      node_impl_type::restore(x->impl(),next->impl(),header()->impl());
      throw;
    }
  }

protected: /* for the benefit of AugmentPolicy::augmented_interface */
  node_type* header()const{return this->final_header();}
  node_type* root()const{return node_type::from_impl(header()->parent());}
  node_type* leftmost()const{return node_type::from_impl(header()->left());}
  node_type* rightmost()const{return node_type::from_impl(header()->right());}

private:
  void empty_initialize()
  {
    header()->color()=red;
    /* used to distinguish header() from root, in iterator.operator++ */

    header()->parent()=node_impl_pointer(0);
    header()->left()=header()->impl();
    header()->right()=header()->impl();
  }

  struct link_info
  {
    /* coverity[uninit_ctor]: suppress warning */
    link_info():side(to_left){}

    ordered_index_side side;
    node_impl_pointer  pos;
  };

  bool link_point(key_param_type k,link_info& inf,ordered_unique_tag)
  {
    node_type* y=header();
    node_type* x=root();
    bool c=true;
    while(x){
      y=x;
      c=comp_(k,key(x->value()));
      x=node_type::from_impl(c?x->left():x->right());
    }
    node_type* yy=y;
    if(c){
      if(yy==leftmost()){
        inf.side=to_left;
        inf.pos=y->impl();
        return true;
      }
      else node_type::decrement(yy);
    }

    if(comp_(key(yy->value()),k)){
      inf.side=c?to_left:to_right;
      inf.pos=y->impl();
      return true;
    }
    else{
      inf.pos=yy->impl();
      return false;
    }
  }

  bool link_point(key_param_type k,link_info& inf,ordered_non_unique_tag)
  {
    node_type* y=header();
    node_type* x=root();
    bool c=true;
    while (x){
     y=x;
     c=comp_(k,key(x->value()));
     x=node_type::from_impl(c?x->left():x->right());
    }
    inf.side=c?to_left:to_right;
    inf.pos=y->impl();
    return true;
  }

  bool lower_link_point(key_param_type k,link_info& inf,ordered_non_unique_tag)
  {
    node_type* y=header();
    node_type* x=root();
    bool c=false;
    while (x){
     y=x;
     c=comp_(key(x->value()),k);
     x=node_type::from_impl(c?x->right():x->left());
    }
    inf.side=c?to_right:to_left;
    inf.pos=y->impl();
    return true;
  }

  bool hinted_link_point(
    key_param_type k,node_type* position,link_info& inf,ordered_unique_tag)
  {
    if(position->impl()==header()->left()){
      if(size()>0&&comp_(k,key(position->value()))){
        inf.side=to_left;
        inf.pos=position->impl();
        return true;
      }
      else return link_point(k,inf,ordered_unique_tag());
    }
    else if(position==header()){
      if(comp_(key(rightmost()->value()),k)){
        inf.side=to_right;
        inf.pos=rightmost()->impl();
        return true;
      }
      else return link_point(k,inf,ordered_unique_tag());
    }
    else{
      node_type* before=position;
      node_type::decrement(before);
      if(comp_(key(before->value()),k)&&comp_(k,key(position->value()))){
        if(before->right()==node_impl_pointer(0)){
          inf.side=to_right;
          inf.pos=before->impl();
          return true;
        }
        else{
          inf.side=to_left;
          inf.pos=position->impl();
          return true;
        }
      }
      else return link_point(k,inf,ordered_unique_tag());
    }
  }

  bool hinted_link_point(
    key_param_type k,node_type* position,link_info& inf,ordered_non_unique_tag)
  {
    if(position->impl()==header()->left()){
      if(size()>0&&!comp_(key(position->value()),k)){
        inf.side=to_left;
        inf.pos=position->impl();
        return true;
      }
      else return lower_link_point(k,inf,ordered_non_unique_tag());
    }
    else if(position==header()){
      if(!comp_(k,key(rightmost()->value()))){
        inf.side=to_right;
        inf.pos=rightmost()->impl();
        return true;
      }
      else return link_point(k,inf,ordered_non_unique_tag());
    }
    else{
      node_type* before=position;
      node_type::decrement(before);
      if(!comp_(k,key(before->value()))){
        if(!comp_(key(position->value()),k)){
          if(before->right()==node_impl_pointer(0)){
            inf.side=to_right;
            inf.pos=before->impl();
            return true;
          }
          else{
            inf.side=to_left;
            inf.pos=position->impl();
            return true;
          }
        }
        else return lower_link_point(k,inf,ordered_non_unique_tag());
      }
      else return link_point(k,inf,ordered_non_unique_tag());
    }
  }

  void delete_all_nodes(node_type* x)
  {
    if(!x)return;

    delete_all_nodes(node_type::from_impl(x->left()));
    delete_all_nodes(node_type::from_impl(x->right()));
    this->final_delete_node_(static_cast<final_node_type*>(x));
  }

  bool in_place(value_param_type v,node_type* x,ordered_unique_tag)
  {
    node_type* y;
    if(x!=leftmost()){
      y=x;
      node_type::decrement(y);
      if(!comp_(key(y->value()),key(v)))return false;
    }

    y=x;
    node_type::increment(y);
    return y==header()||comp_(key(v),key(y->value()));
  }

  bool in_place(value_param_type v,node_type* x,ordered_non_unique_tag)
  {
    node_type* y;
    if(x!=leftmost()){
      y=x;
      node_type::decrement(y);
      if(comp_(key(v),key(y->value())))return false;
    }

    y=x;
    node_type::increment(y);
    return y==header()||!comp_(key(y->value()),key(v));
  }

  template<typename... Args>
  std::pair<iterator,bool> emplace_impl(Args&&... args)
  {
    std::pair<final_node_type*,bool>p=
      this->final_emplace_(std::forward<Args>(args)...);
    return std::pair<iterator,bool>(make_iterator(p.first),p.second);
  }

  template<typename... Args>
  iterator emplace_hint_impl(iterator position,Args&&... args)
  {
    std::pair<final_node_type*,bool>p=
      this->final_emplace_hint_(
        static_cast<final_node_type*>(position.get_node()),
        std::forward<Args>(args)...);
    return make_iterator(p.first);
  }

  template<typename LowerBounder,typename UpperBounder>
  std::pair<iterator,iterator>
  range(LowerBounder lower,UpperBounder upper,none_unbounded_tag)const
  {
    node_type* y=header();
    node_type* z=root();

    while(z){
      if(!lower(key(z->value()))){
        z=node_type::from_impl(z->right());
      }
      else if(!upper(key(z->value()))){
        y=z;
        z=node_type::from_impl(z->left());
      }
      else{
        return std::pair<iterator,iterator>(
          make_iterator(
            lower_range(node_type::from_impl(z->left()),z,lower)),
          make_iterator(
            upper_range(node_type::from_impl(z->right()),y,upper)));
      }
    }

    return std::pair<iterator,iterator>(make_iterator(y),make_iterator(y));
  }

  template<typename LowerBounder,typename UpperBounder>
  std::pair<iterator,iterator>
  range(LowerBounder,UpperBounder upper,lower_unbounded_tag)const
  {
    return std::pair<iterator,iterator>(
      begin(),
      make_iterator(upper_range(root(),header(),upper)));
  }

  template<typename LowerBounder,typename UpperBounder>
  std::pair<iterator,iterator>
  range(LowerBounder lower,UpperBounder,upper_unbounded_tag)const
  {
    return std::pair<iterator,iterator>(
      make_iterator(lower_range(root(),header(),lower)),
      end());
  }

  template<typename LowerBounder,typename UpperBounder>
  std::pair<iterator,iterator>
  range(LowerBounder,UpperBounder,both_unbounded_tag)const
  {
    return std::pair<iterator,iterator>(begin(),end());
  }

  template<typename LowerBounder>
  node_type * lower_range(node_type* top,node_type* y,LowerBounder lower)const
  {
    while(top){
      if(lower(key(top->value()))){
        y=top;
        top=node_type::from_impl(top->left());
      }
      else top=node_type::from_impl(top->right());
    }

    return y;
  }

  template<typename UpperBounder>
  node_type * upper_range(node_type* top,node_type* y,UpperBounder upper)const
  {
    while(top){
      if(!upper(key(top->value()))){
        y=top;
        top=node_type::from_impl(top->left());
      }
      else top=node_type::from_impl(top->right());
    }

    return y;
  }

protected: /* for the benefit of AugmentPolicy::augmented_interface */
  key_from_value key;
  key_compare    comp_;
};

template<
  typename KeyFromValue,typename Compare,
  typename SuperMeta,typename TagList,typename Category,typename AugmentPolicy
>
class ordered_index:
  public AugmentPolicy::template augmented_interface<
    ordered_index_impl<
      KeyFromValue,Compare,SuperMeta,TagList,Category,AugmentPolicy
    >
  >::type
{
  typedef typename AugmentPolicy::template
    augmented_interface<
      ordered_index_impl<
        KeyFromValue,Compare,
        SuperMeta,TagList,Category,AugmentPolicy
      >
    >::type                                       super;
public:
  typedef typename super::ctor_args_list          ctor_args_list;
  typedef typename super::allocator_type          allocator_type;
  typedef typename super::iterator                iterator;

  /* construct/copy/destroy
   * Default and copy ctors are in the protected section as indices are
   * not supposed to be created on their own. No range ctor either.
   */

  ordered_index& operator=(const ordered_index& x)
  {
    this->final()=x.final();
    return *this;
  }

  ordered_index& operator=(
    std::initializer_list<typename super::value_type> list)
  {
    this->final()=list;
    return *this;
  }

protected:
  ordered_index(
    const ctor_args_list& args_list,const allocator_type& al):
    super(args_list,al){}

  ordered_index(const ordered_index& x):super(x){};

  ordered_index(const ordered_index& x,do_not_copy_elements_tag):
    super(x,do_not_copy_elements_tag()){};
};

/* comparison */

template<
  typename KeyFromValue1,typename Compare1,
  typename SuperMeta1,typename TagList1,typename Category1,
  typename AugmentPolicy1,
  typename KeyFromValue2,typename Compare2,
  typename SuperMeta2,typename TagList2,typename Category2,
  typename AugmentPolicy2
>
bool operator==(
  const ordered_index<
    KeyFromValue1,Compare1,SuperMeta1,TagList1,Category1,AugmentPolicy1>& x,
  const ordered_index<
    KeyFromValue2,Compare2,SuperMeta2,TagList2,Category2,AugmentPolicy2>& y)
{
  return x.size()==y.size()&&std::equal(x.begin(),x.end(),y.begin());
}

template<
  typename KeyFromValue1,typename Compare1,
  typename SuperMeta1,typename TagList1,typename Category1,
  typename AugmentPolicy1,
  typename KeyFromValue2,typename Compare2,
  typename SuperMeta2,typename TagList2,typename Category2,
  typename AugmentPolicy2
>
bool operator<(
  const ordered_index<
    KeyFromValue1,Compare1,SuperMeta1,TagList1,Category1,AugmentPolicy1>& x,
  const ordered_index<
    KeyFromValue2,Compare2,SuperMeta2,TagList2,Category2,AugmentPolicy2>& y)
{
  return std::lexicographical_compare(x.begin(),x.end(),y.begin(),y.end());
}

template<
  typename KeyFromValue1,typename Compare1,
  typename SuperMeta1,typename TagList1,typename Category1,
  typename AugmentPolicy1,
  typename KeyFromValue2,typename Compare2,
  typename SuperMeta2,typename TagList2,typename Category2,
  typename AugmentPolicy2
>
bool operator!=(
  const ordered_index<
    KeyFromValue1,Compare1,SuperMeta1,TagList1,Category1,AugmentPolicy1>& x,
  const ordered_index<
    KeyFromValue2,Compare2,SuperMeta2,TagList2,Category2,AugmentPolicy2>& y)
{
  return !(x==y);
}

template<
  typename KeyFromValue1,typename Compare1,
  typename SuperMeta1,typename TagList1,typename Category1,
  typename AugmentPolicy1,
  typename KeyFromValue2,typename Compare2,
  typename SuperMeta2,typename TagList2,typename Category2,
  typename AugmentPolicy2
>
bool operator>(
  const ordered_index<
    KeyFromValue1,Compare1,SuperMeta1,TagList1,Category1,AugmentPolicy1>& x,
  const ordered_index<
    KeyFromValue2,Compare2,SuperMeta2,TagList2,Category2,AugmentPolicy2>& y)
{
  return y<x;
}

template<
  typename KeyFromValue1,typename Compare1,
  typename SuperMeta1,typename TagList1,typename Category1,
  typename AugmentPolicy1,
  typename KeyFromValue2,typename Compare2,
  typename SuperMeta2,typename TagList2,typename Category2,
  typename AugmentPolicy2
>
bool operator>=(
  const ordered_index<
    KeyFromValue1,Compare1,SuperMeta1,TagList1,Category1,AugmentPolicy1>& x,
  const ordered_index<
    KeyFromValue2,Compare2,SuperMeta2,TagList2,Category2,AugmentPolicy2>& y)
{
  return !(x<y);
}

template<
  typename KeyFromValue1,typename Compare1,
  typename SuperMeta1,typename TagList1,typename Category1,
  typename AugmentPolicy1,
  typename KeyFromValue2,typename Compare2,
  typename SuperMeta2,typename TagList2,typename Category2,
  typename AugmentPolicy2
>
bool operator<=(
  const ordered_index<
    KeyFromValue1,Compare1,SuperMeta1,TagList1,Category1,AugmentPolicy1>& x,
  const ordered_index<
    KeyFromValue2,Compare2,SuperMeta2,TagList2,Category2,AugmentPolicy2>& y)
{
  return !(x>y);
}

/*  specialized algorithms */

template<
  typename KeyFromValue,typename Compare,
  typename SuperMeta,typename TagList,typename Category,typename AugmentPolicy
>
void swap(
  ordered_index<
    KeyFromValue,Compare,SuperMeta,TagList,Category,AugmentPolicy>& x,
  ordered_index<
    KeyFromValue,Compare,SuperMeta,TagList,Category,AugmentPolicy>& y)
{
  x.swap(y);
}

} /* namespace multi_index::detail */

} /* namespace multi_index */
