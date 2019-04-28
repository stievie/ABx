/* Multiply indexed container.
 *
 * Copyright 2003-2014 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

#include <brigand/algorithms/find.hpp>
#include <brigand/sequences/at.hpp>
#include <brigand/sequences/front.hpp>
#include <brigand/sequences/size.hpp>
#include <algorithm>
#include <multi_index_container_fwd.hpp>
#include <multi_index/detail/base_from_member.hpp>
#include <multi_index/detail/base_type.hpp>
#include <multi_index/detail/do_not_copy_elements_tag.hpp>
#include <multi_index/detail/converter.hpp>
#include <multi_index/detail/header_holder.hpp>
#include <multi_index/detail/has_tag.hpp>
#include <multi_index/detail/no_duplicate_tags.hpp>
#include <multi_index/detail/scope_guard.hpp>
#include <multi_index/detail/vartempl_support.hpp>
#include <initializer_list>
#include <utility>

namespace multi_index{

template<typename Value,typename IndexSpecifierList,typename Allocator>
class multi_index_container:
  private ::multi_index::base_from_member<
    typename std::allocator_traits<Allocator>::template rebind_alloc<
      typename detail::multi_index_node_type<
        Value,
        IndexSpecifierList,
        Allocator
      >::type
    >>,
  private detail::header_holder<
    typename std::allocator_traits<Allocator>::template rebind_traits<
      typename detail::multi_index_node_type<
        Value,
        IndexSpecifierList,
        Allocator
      >::type
    >::pointer,
    multi_index_container<Value,IndexSpecifierList,Allocator> >,

  public detail::multi_index_base_type<
    Value,
    IndexSpecifierList,
    Allocator
  >::type
{
private:
  template <typename,typename,typename> friend class  detail::index_base;
  template <typename,typename>          friend struct detail::header_holder;
  template <typename,typename>          friend struct detail::converter;

  typedef typename detail::multi_index_base_type<
      Value,
      IndexSpecifierList,
      Allocator
  >::type                                         super;

  typedef typename
  std::allocator_traits<Allocator>::template rebind_alloc<
    typename super::node_type
  >                                               node_allocator;
  typedef ::multi_index::base_from_member<
    node_allocator>                               bfm_allocator;
  typedef detail::header_holder<
    typename std::allocator_traits<node_allocator>::pointer,
    multi_index_container>                        bfm_header;

public:
  /* All types are inherited from super, a few are explicitly
   * brought forward here to save us some typename's.
   */

  typedef typename super::ctor_args_list           ctor_args_list;
  typedef IndexSpecifierList                       index_specifier_type_list;

  typedef typename super::index_type_list          index_type_list;

  typedef typename super::iterator_type_list       iterator_type_list;
  typedef typename super::const_iterator_type_list const_iterator_type_list;
  typedef typename super::value_type               value_type;
  typedef typename super::final_allocator_type     allocator_type;
  typedef typename super::iterator                 iterator;
  typedef typename super::const_iterator           const_iterator;

  static_assert(detail::no_duplicate_tags_in_index_list<index_type_list>::value, "");

  /* global project() needs to see this publicly */

  typedef typename super::node_type node_type;

  /* construct/copy/destroy */

  explicit multi_index_container(
    const ctor_args_list& args_list=ctor_args_list(),
    const allocator_type& al=allocator_type()):

    bfm_allocator(al),
    super(args_list,bfm_allocator::member),
    node_count(0)
  {
  }

  explicit multi_index_container(const allocator_type& al):
    bfm_allocator(al),
    super(ctor_args_list(),bfm_allocator::member),
    node_count(0)
  {
  }

  template<typename InputIterator>
  multi_index_container(InputIterator first,InputIterator last,

    const ctor_args_list& args_list=ctor_args_list(),
    const allocator_type& al=allocator_type()):

    bfm_allocator(al),
    super(args_list,bfm_allocator::member),
    node_count(0)
  {
    try {
      iterator hint=super::end();
      for(;first!=last;++first){
        hint=super::make_iterator(
          insert_ref_(*first,hint.get_node()).first);
        ++hint;
      }
    }
    catch(...){
      clear_();
      throw;
    }
  }

  multi_index_container(
    std::initializer_list<Value> list,
    const ctor_args_list& args_list=ctor_args_list(),
    const allocator_type& al=allocator_type()):
    bfm_allocator(al),
    super(args_list,bfm_allocator::member),
    node_count(0)
  {
    try {
      typedef const Value* init_iterator;

      iterator hint=super::end();
      for(init_iterator first=list.begin(),last=list.end();
          first!=last;++first){
        hint=super::make_iterator(insert_(*first,hint.get_node()).first);
        ++hint;
      }
    }
    catch(...){
      clear_();
      throw;
    }
  }

  multi_index_container(
    const multi_index_container<Value,IndexSpecifierList,Allocator>& x):
    bfm_allocator(x.bfm_allocator::member),
    bfm_header(),
    super(x),
    node_count(0)
  {
    copy_map_type map(bfm_allocator::member,x.size(),x.header(),header());
    for(const_iterator it=x.begin(),it_end=x.end();it!=it_end;++it){
      map.clone(it.get_node());
    }
    super::copy_(x,map);
    map.release();
    node_count=x.size();

    /* Not until this point are the indices required to be consistent,
     * hence the position of the invariant checker.
     */

  }

  multi_index_container(multi_index_container&& x):
    bfm_allocator(x.bfm_allocator::member),
    bfm_header(),
    super(x,detail::do_not_copy_elements_tag()),
    node_count(0)
  {
    swap_elements_(x);
  }

  ~multi_index_container()
  {
    delete_all_nodes_();
  }

  multi_index_container<Value,IndexSpecifierList,Allocator>& operator=(
    const multi_index_container& x)
  {
    multi_index_container y(x);
    this->swap(y);
    return *this;
  }

  multi_index_container<Value,IndexSpecifierList,Allocator>& operator=(
    multi_index_container&& x)
  {
    this->swap(x);
    return *this;
  }

  multi_index_container<Value,IndexSpecifierList,Allocator>& operator=(
    std::initializer_list<Value> list)
  {
    typedef const Value* init_iterator;

    multi_index_container x(*this,detail::do_not_copy_elements_tag());
    iterator hint=x.end();
    for(init_iterator first=list.begin(),last=list.end();
        first!=last;++first){
      hint=x.make_iterator(x.insert_(*first,hint.get_node()).first);
      ++hint;
    }
    x.swap_elements_(*this);
    return*this;
  }

  allocator_type get_allocator()const noexcept
  {
    return allocator_type(bfm_allocator::member);
  }

  /* retrieval of indices by number */

  template<int N>
  struct nth_index
  {
    static_assert(N>=0&&N<brigand::size<index_type_list>::value, "");
    typedef brigand::at_c<index_type_list,N> type;
  };

  template<int N>
  typename nth_index<N>::type& get()noexcept
  {
    static_assert(N>=0&&N<brigand::size<index_type_list>::value, "");
    return *this;
  }

  template<int N>
  const typename nth_index<N>::type& get()const noexcept
  {
    static_assert(N>=0&&N<brigand::size<index_type_list>::value, "");
    return *this;
  }

  /* retrieval of indices by tag */

  template<typename Tag>
  struct index
  {
    template <typename index_t>
    using has_tag = typename detail::has_tag<Tag, typename index_t::tag_list>::type;

    using list = brigand::find<index_type_list, brigand::bind<has_tag, brigand::_1>>;
    using type = brigand::front<list>;
  };

  template<typename Tag>
  typename index<Tag>::type& get()noexcept
  {
    return *this;
  }

  template<typename Tag>
  const typename index<Tag>::type& get()const noexcept
  {
    return *this;
  }

  /* projection of iterators by number */

  template<int N>
  struct nth_index_iterator
  {
    typedef typename nth_index<N>::type::iterator type;
  };

  template<int N>
  struct nth_index_const_iterator
  {
    typedef typename nth_index<N>::type::const_iterator type;
  };

  template<int N,typename IteratorType>
  typename nth_index_iterator<N>::type project(IteratorType it)
  {
    typedef typename nth_index<N>::type index_type;
    return index_type::make_iterator(static_cast<node_type*>(it.get_node()));
  }

  template<int N,typename IteratorType>
  typename nth_index_const_iterator<N>::type project(IteratorType it)const
  {
    typedef typename nth_index<N>::type index_type;
    return index_type::make_iterator(static_cast<node_type*>(it.get_node()));
  }

  /* projection of iterators by tag */

  template<typename Tag>
  struct index_iterator
  {
    typedef typename index<Tag>::type::iterator type;
  };

  template<typename Tag>
  struct index_const_iterator
  {
    typedef typename index<Tag>::type::const_iterator type;
  };

  template<typename Tag,typename IteratorType>
  typename index_iterator<Tag>::type project(IteratorType it)
  {
    typedef typename index<Tag>::type index_type;
    return index_type::make_iterator(static_cast<node_type*>(it.get_node()));
  }

  template<typename Tag,typename IteratorType>
  typename index_const_iterator<Tag>::type project(IteratorType it)const
  {
    typedef typename index<Tag>::type index_type;
    return index_type::make_iterator(static_cast<node_type*>(it.get_node()));
  }

protected:
  typedef typename super::copy_map_type copy_map_type;

  multi_index_container(
    const multi_index_container<Value,IndexSpecifierList,Allocator>& x,
    detail::do_not_copy_elements_tag):
    bfm_allocator(x.bfm_allocator::member),
    bfm_header(),
    super(x,detail::do_not_copy_elements_tag()),
    node_count(0)
  {
  }

  node_type* header()const
  {
    return &*bfm_header::member;
  }

  node_type* allocate_node()
  {
    return &*bfm_allocator::member.allocate(1);
  }

  void deallocate_node(node_type* x)
  {
    typedef typename std::allocator_traits<node_allocator>::pointer node_pointer;
    bfm_allocator::member.deallocate(static_cast<node_pointer>(x),1);
  }

  bool empty_()const
  {
    return node_count==0;
  }

  std::size_t size_()const
  {
    return node_count;
  }

  std::size_t max_size_()const
  {
    return static_cast<std::size_t >(-1);
  }

  template<typename Variant>
  std::pair<node_type*,bool> insert_(const Value& v,Variant variant)
  {
    node_type* x=0;
    node_type* res=super::insert_(v,x,variant);
    if(res==x){
      ++node_count;
      return std::pair<node_type*,bool>(res,true);
    }
    else{
      return std::pair<node_type*,bool>(res,false);
    }
  }

  std::pair<node_type*,bool> insert_(const Value& v)
  {
    return insert_(v,detail::lvalue_tag());
  }

  std::pair<node_type*,bool> insert_rv_(const Value& v)
  {
    return insert_(v,detail::rvalue_tag());
  }

  template<typename T>
  std::pair<node_type*,bool> insert_ref_(T& t)
  {
    node_type* x=allocate_node();
    try {
      new(&x->value()) value_type(t);
      try {
        node_type* res=super::insert_(x->value(),x,detail::emplaced_tag());
        if(res==x){
          ++node_count;
          return std::pair<node_type*,bool>(res,true);
        }
        else{
          multi_index::detail::allocator::destroy(&x->value());
          deallocate_node(x);
          return std::pair<node_type*,bool>(res,false);
        }
      }
      catch(...){
        multi_index::detail::allocator::destroy(&x->value());
        throw;
      }
    }
    catch(...){
      deallocate_node(x);
      throw;
    }
  }

  std::pair<node_type*,bool> insert_ref_(const value_type& x)
  {
    return insert_(x);
  }

  std::pair<node_type*,bool> insert_ref_(value_type& x)
  {
    return insert_(x);
  }

  template<typename... Args>
  std::pair<node_type*,bool> emplace_(Args&&... args)
  {
    node_type* x=allocate_node();
    try {
      detail::vartempl_placement_new(
        &x->value(),std::forward<Args>(args)...);
      try {
        node_type* res=super::insert_(x->value(),x,detail::emplaced_tag());
        if(res==x){
          ++node_count;
          return std::pair<node_type*,bool>(res,true);
        }
        else{
          multi_index::detail::allocator::destroy(&x->value());
          deallocate_node(x);
          return std::pair<node_type*,bool>(res,false);
        }
      }
      catch(...){
        multi_index::detail::allocator::destroy(&x->value());
        throw;
      }
    }
    catch(...){
      deallocate_node(x);
      throw;
    }
  }

  template<typename Variant>
  std::pair<node_type*,bool> insert_(
    const Value& v,node_type* position,Variant variant)
  {
    node_type* x=0;
    node_type* res=super::insert_(v,position,x,variant);
    if(res==x){
      ++node_count;
      return std::pair<node_type*,bool>(res,true);
    }
    else{
      return std::pair<node_type*,bool>(res,false);
    }
  }

  std::pair<node_type*,bool> insert_(const Value& v,node_type* position)
  {
    return insert_(v,position,detail::lvalue_tag());
  }

  std::pair<node_type*,bool> insert_rv_(const Value& v,node_type* position)
  {
    return insert_(v,position,detail::rvalue_tag());
  }

  template<typename T>
  std::pair<node_type*,bool> insert_ref_(
    T& t,node_type* position)
  {
    node_type* x=allocate_node();
    try {
      new(&x->value()) value_type(t);
      try {
        node_type* res=super::insert_(
          x->value(),position,x,detail::emplaced_tag());
        if(res==x){
          ++node_count;
          return std::pair<node_type*,bool>(res,true);
        }
        else{
          multi_index::detail::allocator::destroy(&x->value());
          deallocate_node(x);
          return std::pair<node_type*,bool>(res,false);
        }
      }
      catch(...){
        multi_index::detail::allocator::destroy(&x->value());
        throw;
      }
    }
    catch(...){
      deallocate_node(x);
      throw;
    }
  }

  std::pair<node_type*,bool> insert_ref_(
    const value_type& x,node_type* position)
  {
    return insert_(x,position);
  }

  std::pair<node_type*,bool> insert_ref_(
    value_type& x,node_type* position)
  {
    return insert_(x,position);
  }

  template<typename... Args>
  std::pair<node_type*,bool> emplace_hint_(node_type* position,Args&&... args)
  {
    node_type* x=allocate_node();
    try {
      detail::vartempl_placement_new(
        &x->value(),std::forward<Args>(args)...);
      try {
        node_type* res=super::insert_(
          x->value(),position,x,detail::emplaced_tag());
        if(res==x){
          ++node_count;
          return std::pair<node_type*,bool>(res,true);
        }
        else{
          multi_index::detail::allocator::destroy(&x->value());
          deallocate_node(x);
          return std::pair<node_type*,bool>(res,false);
        }
      }
      catch(...){
        multi_index::detail::allocator::destroy(&x->value());
        throw;
      }
    }
    catch(...){
      deallocate_node(x);
      throw;
    }
  }

  void erase_(node_type* x)
  {
    --node_count;
    super::erase_(x);
    deallocate_node(x);
  }

  void delete_node_(node_type* x)
  {
    super::delete_node_(x);
    deallocate_node(x);
  }

  void delete_all_nodes_()
  {
    super::delete_all_nodes_();
  }

  void clear_()
  {
    delete_all_nodes_();
    super::clear_();
    node_count=0;
  }

  void swap_(multi_index_container<Value,IndexSpecifierList,Allocator>& x)
  {
    if(bfm_allocator::member!=x.bfm_allocator::member){
      std::swap(bfm_allocator::member,x.bfm_allocator::member);
    }
    std::swap(bfm_header::member,x.bfm_header::member);
    super::swap_(x);
    std::swap(node_count,x.node_count);
  }

  void swap_elements_(
    multi_index_container<Value,IndexSpecifierList,Allocator>& x)
  {
    std::swap(bfm_header::member,x.bfm_header::member);
    super::swap_elements_(x);
    std::swap(node_count,x.node_count);
  }

  bool replace_(const Value& k,node_type* x)
  {
    return super::replace_(k,x,detail::lvalue_tag());
  }

  bool replace_rv_(const Value& k,node_type* x)
  {
    return super::replace_(k,x,detail::rvalue_tag());
  }

  template<typename Modifier>
  bool modify_(Modifier& mod,node_type* x)
  {
    mod(const_cast<value_type&>(x->value()));

    try {
      if(!super::modify_(x)){
        deallocate_node(x);
        --node_count;
        return false;
      }
      else return true;
    }
    catch(...){
      deallocate_node(x);
      --node_count;
      throw;
    }
  }

  template<typename Modifier,typename Rollback>
  bool modify_(Modifier& mod,Rollback& back_,node_type* x)
  {
    mod(const_cast<value_type&>(x->value()));

    bool b;
    try {
      b=super::modify_rollback_(x);
    }
    catch(...){
      try {
        back_(const_cast<value_type&>(x->value()));
        throw;
      }
      catch(...){
        this->erase_(x);
        throw;
      }
    }

    try {
      if(!b){
        back_(const_cast<value_type&>(x->value()));
        return false;
      }
      else return true;
    }
    catch(...){
      this->erase_(x);
      throw;
    }
  }

private:
  std::size_t node_count;
};

/* retrieval of indices by number */

template<typename MultiIndexContainer,int N>
struct nth_index
{
  static const int M=brigand::size<typename MultiIndexContainer::index_type_list>::value;
  static_assert(N>=0&&N<M, "");
  typedef brigand::at_c<
    typename MultiIndexContainer::index_type_list,N> type;
};

template<int N,typename Value,typename IndexSpecifierList,typename Allocator>
typename nth_index<
  multi_index_container<Value,IndexSpecifierList,Allocator>,N>::type&
get(
  multi_index_container<Value,IndexSpecifierList,Allocator>& m)noexcept
{
  typedef multi_index_container<
    Value,IndexSpecifierList,Allocator>    multi_index_type;
  typedef typename nth_index<
    multi_index_container<
      Value,IndexSpecifierList,Allocator>,
    N
  >::type                                  index_type;

  static_assert(N>=0&&
    N<
    brigand::size<
      typename multi_index_type::index_type_list
    >::value, "");

  return detail::converter<multi_index_type,index_type>::index(m);
}

template<int N,typename Value,typename IndexSpecifierList,typename Allocator>
const typename nth_index<
  multi_index_container<Value,IndexSpecifierList,Allocator>,N>::type&
get(
  const multi_index_container<Value,IndexSpecifierList,Allocator>& m
)noexcept
{
  typedef multi_index_container<
    Value,IndexSpecifierList,Allocator>    multi_index_type;
  typedef typename nth_index<
    multi_index_container<
      Value,IndexSpecifierList,Allocator>,
    N
  >::type                                  index_type;

  static_assert(N>=0&&
    N<
    brigand::size<
      typename multi_index_type::index_type_list
    >::value, "");

  return detail::converter<multi_index_type,index_type>::index(m);
}

/* retrieval of indices by tag */

template<typename MultiIndexContainer,typename Tag>
struct index
{
  typedef typename MultiIndexContainer::index_type_list index_type_list;

  template <typename index_t>
  using has_tag = typename detail::has_tag<Tag, typename index_t::tag_list>::type;

  using list = brigand::find<index_type_list, brigand::bind<has_tag, brigand::_1>>;
  using type = brigand::front<list>;
};

template<
  typename Tag,typename Value,typename IndexSpecifierList,typename Allocator
>
typename ::multi_index::index<
  multi_index_container<Value,IndexSpecifierList,Allocator>,Tag>::type&
get(
  multi_index_container<Value,IndexSpecifierList,Allocator>& m)noexcept
{
  typedef multi_index_container<
    Value,IndexSpecifierList,Allocator>         multi_index_type;
  typedef typename ::multi_index::index<
    multi_index_container<
      Value,IndexSpecifierList,Allocator>,
    Tag
  >::type                                       index_type;

  return detail::converter<multi_index_type,index_type>::index(m);
}

template<
  typename Tag,typename Value,typename IndexSpecifierList,typename Allocator
>
const typename ::multi_index::index<
  multi_index_container<Value,IndexSpecifierList,Allocator>,Tag>::type&
get(
  const multi_index_container<Value,IndexSpecifierList,Allocator>& m
)noexcept
{
  typedef multi_index_container<
    Value,IndexSpecifierList,Allocator>         multi_index_type;
  typedef typename ::multi_index::index<
    multi_index_container<
      Value,IndexSpecifierList,Allocator>,
    Tag
  >::type                                       index_type;

  return detail::converter<multi_index_type,index_type>::index(m);
}

/* projection of iterators by number */

template<typename MultiIndexContainer,int N>
struct nth_index_iterator
{
  typedef typename nth_index<MultiIndexContainer,N>::type::iterator type;
};

template<typename MultiIndexContainer,int N>
struct nth_index_const_iterator
{
  typedef typename nth_index<MultiIndexContainer,N>::type::const_iterator type;
};

template<
  int N,typename IteratorType,
  typename Value,typename IndexSpecifierList,typename Allocator>
typename nth_index_iterator<
  multi_index_container<Value,IndexSpecifierList,Allocator>,N>::type
project(
  multi_index_container<Value,IndexSpecifierList,Allocator>& m,
  IteratorType it)
{
  typedef multi_index_container<
    Value,IndexSpecifierList,Allocator>                multi_index_type;
  typedef typename nth_index<multi_index_type,N>::type index_type;

  return detail::converter<multi_index_type,index_type>::iterator(
    m,static_cast<typename multi_index_type::node_type*>(it.get_node()));
}

template<
  int N,typename IteratorType,
  typename Value,typename IndexSpecifierList,typename Allocator>
typename nth_index_const_iterator<
  multi_index_container<Value,IndexSpecifierList,Allocator>,N>::type
project(
  const multi_index_container<Value,IndexSpecifierList,Allocator>& m,
  IteratorType it)
{
  typedef multi_index_container<
    Value,IndexSpecifierList,Allocator>                multi_index_type;
  typedef typename nth_index<multi_index_type,N>::type index_type;

  return detail::converter<multi_index_type,index_type>::const_iterator(
    m,static_cast<typename multi_index_type::node_type*>(it.get_node()));
}

/* projection of iterators by tag */

template<typename MultiIndexContainer,typename Tag>
struct index_iterator
{
  typedef typename ::multi_index::index<
    MultiIndexContainer,Tag>::type::iterator    type;
};

template<typename MultiIndexContainer,typename Tag>
struct index_const_iterator
{
  typedef typename ::multi_index::index<
    MultiIndexContainer,Tag>::type::const_iterator type;
};

template<
  typename Tag,typename IteratorType,
  typename Value,typename IndexSpecifierList,typename Allocator>
typename index_iterator<
  multi_index_container<Value,IndexSpecifierList,Allocator>,Tag>::type
project(
  multi_index_container<Value,IndexSpecifierList,Allocator>& m,
  IteratorType it)
{
  typedef multi_index_container<
    Value,IndexSpecifierList,Allocator>         multi_index_type;
  typedef typename ::multi_index::index<
    multi_index_type,Tag>::type                 index_type;

  return detail::converter<multi_index_type,index_type>::iterator(
    m,static_cast<typename multi_index_type::node_type*>(it.get_node()));
}

template<
  typename Tag,typename IteratorType,
  typename Value,typename IndexSpecifierList,typename Allocator>
typename index_const_iterator<
  multi_index_container<Value,IndexSpecifierList,Allocator>,Tag>::type
project(
  const multi_index_container<Value,IndexSpecifierList,Allocator>& m,
  IteratorType it)
{
  typedef multi_index_container<
    Value,IndexSpecifierList,Allocator>         multi_index_type;
  typedef typename ::multi_index::index<
    multi_index_type,Tag>::type                 index_type;

  return detail::converter<multi_index_type,index_type>::const_iterator(
    m,static_cast<typename multi_index_type::node_type*>(it.get_node()));
}

/* Comparison. Simple forward to first index. */

template<
  typename Value1,typename IndexSpecifierList1,typename Allocator1,
  typename Value2,typename IndexSpecifierList2,typename Allocator2
>
bool operator==(
  const multi_index_container<Value1,IndexSpecifierList1,Allocator1>& x,
  const multi_index_container<Value2,IndexSpecifierList2,Allocator2>& y)
{
  return get<0>(x)==get<0>(y);
}

template<
  typename Value1,typename IndexSpecifierList1,typename Allocator1,
  typename Value2,typename IndexSpecifierList2,typename Allocator2
>
bool operator<(
  const multi_index_container<Value1,IndexSpecifierList1,Allocator1>& x,
  const multi_index_container<Value2,IndexSpecifierList2,Allocator2>& y)
{
  return get<0>(x)<get<0>(y);
}

template<
  typename Value1,typename IndexSpecifierList1,typename Allocator1,
  typename Value2,typename IndexSpecifierList2,typename Allocator2
>
bool operator!=(
  const multi_index_container<Value1,IndexSpecifierList1,Allocator1>& x,
  const multi_index_container<Value2,IndexSpecifierList2,Allocator2>& y)
{
  return get<0>(x)!=get<0>(y);
}

template<
  typename Value1,typename IndexSpecifierList1,typename Allocator1,
  typename Value2,typename IndexSpecifierList2,typename Allocator2
>
bool operator>(
  const multi_index_container<Value1,IndexSpecifierList1,Allocator1>& x,
  const multi_index_container<Value2,IndexSpecifierList2,Allocator2>& y)
{
  return get<0>(x)>get<0>(y);
}

template<
  typename Value1,typename IndexSpecifierList1,typename Allocator1,
  typename Value2,typename IndexSpecifierList2,typename Allocator2
>
bool operator>=(
  const multi_index_container<Value1,IndexSpecifierList1,Allocator1>& x,
  const multi_index_container<Value2,IndexSpecifierList2,Allocator2>& y)
{
  return get<0>(x)>=get<0>(y);
}

template<
  typename Value1,typename IndexSpecifierList1,typename Allocator1,
  typename Value2,typename IndexSpecifierList2,typename Allocator2
>
bool operator<=(
  const multi_index_container<Value1,IndexSpecifierList1,Allocator1>& x,
  const multi_index_container<Value2,IndexSpecifierList2,Allocator2>& y)
{
  return get<0>(x)<=get<0>(y);
}

/*  specialized algorithms */

template<typename Value,typename IndexSpecifierList,typename Allocator>
void swap(
  multi_index_container<Value,IndexSpecifierList,Allocator>& x,
  multi_index_container<Value,IndexSpecifierList,Allocator>& y)
{
  x.swap(y);
}

} /* namespace multi_index */
