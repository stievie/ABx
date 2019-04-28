/* Copyright 2003-2015 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

#include <brigand/functions/logical/not.hpp>
#include <brigand/sequences/front.hpp>
#include <brigand/types/bool.hpp>
#include <multi_index/detail/call_traits.hpp>
#include <multi_index/detail/allocator_utilities.hpp>
#include <multi_index/detail/bidir_node_iterator.hpp>
#include <multi_index/detail/do_not_copy_elements_tag.hpp>
#include <multi_index/detail/index_node_base.hpp>
#include <multi_index/detail/scope_guard.hpp>
#include <multi_index/detail/seq_index_node.hpp>
#include <multi_index/detail/seq_index_ops.hpp>
#include <multi_index/detail/vartempl_support.hpp>
#include <multi_index/config.hpp>
#include <multi_index/sequenced_index_fwd.hpp>
#include <multi_index/tuple.hpp>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <functional>
#include <type_traits>
#include <utility>

namespace multi_index{

namespace detail{

/* sequenced_index adds a layer of sequenced indexing to a given Super */

template<typename SuperMeta,typename TagList>
class sequenced_index:
  protected SuperMeta::type

{
  typedef typename SuperMeta::type                    super;

protected:
  typedef sequenced_index_node<
    typename super::node_type>                        node_type;

private:
  typedef typename node_type::impl_type               node_impl_type;

public:
  /* types */

  typedef typename node_type::value_type              value_type;
  typedef tuples::null_type                           ctor_args;
  typedef typename super::final_allocator_type        allocator_type;
  typedef typename std::allocator_traits<
    allocator_type>::value_type&                      reference;
  typedef typename std::allocator_traits<
    allocator_type>::value_type const&                const_reference;
  typedef bidir_node_iterator<node_type>              iterator;
  typedef iterator                                    const_iterator;

  typedef std::size_t                                 size_type;
  typedef std::ptrdiff_t                              difference_type;
  typedef typename std::allocator_traits<
    allocator_type>::pointer                          pointer;
  typedef typename std::allocator_traits<
    allocator_type>::const_pointer                    const_pointer;
  typedef typename
    std::reverse_iterator<iterator>                 reverse_iterator;
  typedef typename
    std::reverse_iterator<const_iterator>           const_reverse_iterator;
  typedef TagList                                     tag_list;

protected:
  typedef typename super::final_node_type     final_node_type;
  typedef tuples::cons<
    ctor_args,
    typename super::ctor_args_list>           ctor_args_list;
  typedef brigand::push_front<
    typename super::index_type_list,
    sequenced_index>                          index_type_list;
  typedef brigand::push_front<
    typename super::iterator_type_list,
    iterator>                                 iterator_type_list;
  typedef brigand::push_front<
    typename super::const_iterator_type_list,
    const_iterator>                           const_iterator_type_list;
  typedef typename super::copy_map_type       copy_map_type;

private:
  typedef typename call_traits<value_type>::param_type value_param_type;

  /* Needed to avoid commas in MULTI_INDEX_OVERLOADS_TO_VARTEMPL
   * expansion.
   */

  typedef std::pair<iterator,bool>                     emplace_return_type;

public:

  /* construct/copy/destroy
   * Default and copy ctors are in the protected section as indices are
   * not supposed to be created on their own. No range ctor either.
   */

  sequenced_index<SuperMeta,TagList>& operator=(
    const sequenced_index<SuperMeta,TagList>& x)
  {
    this->final()=x.final();
    return *this;
  }

  sequenced_index<SuperMeta,TagList>& operator=(
    std::initializer_list<value_type> list)
  {
    this->final()=list;
    return *this;
  }

  template <class InputIterator>
  void assign(InputIterator first,InputIterator last)
  {
    assign_iter(first,last,brigand::not_<std::is_integral<InputIterator> >());
  }

  void assign(std::initializer_list<value_type> list)
  {
    assign(list.begin(),list.end());
  }

  void assign(size_type n,value_param_type value)
  {
    clear();
    for(size_type i=0;i<n;++i)push_back(value);
  }

  allocator_type get_allocator()const noexcept
  {
    return this->final().get_allocator();
  }

  /* iterators */

  iterator  begin()noexcept
    {return make_iterator(node_type::from_impl(header()->next()));}
  const_iterator begin()const noexcept
    {return make_iterator(node_type::from_impl(header()->next()));}
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

  void resize(size_type n)
  {
    if(n>size()){
      for(size_type m=n-size();m--;)
        this->final_emplace_();
    }
    else if(n<size()){for(size_type m=size()-n;m--;)pop_back();}
  }

  void resize(size_type n,value_param_type x)
  {
    if(n>size())insert(end(),n-size(),x);
    else if(n<size())for(size_type m=size()-n;m--;)pop_back();
  }

  /* access: no non-const versions provided as sequenced_index
   * handles const elements.
   */

  const_reference front()const{return *begin();}
  const_reference back()const{return *--end();}

  /* modifiers */

  MULTI_INDEX_OVERLOADS_TO_VARTEMPL(
    emplace_return_type,emplace_front,emplace_front_impl)

  std::pair<iterator,bool> push_front(const value_type& x)
                             {return insert(begin(),x);}
  std::pair<iterator,bool> push_front(value_type&& x)
                             {return insert(begin(),std::move(x));}
  void                     pop_front(){erase(begin());}

  MULTI_INDEX_OVERLOADS_TO_VARTEMPL(
    emplace_return_type,emplace_back,emplace_back_impl)

  std::pair<iterator,bool> push_back(const value_type& x)
                             {return insert(end(),x);}
  std::pair<iterator,bool> push_back(value_type&& x)
                             {return insert(end(),std::move(x));}
  void                     pop_back(){erase(--end());}

  MULTI_INDEX_OVERLOADS_TO_VARTEMPL_EXTRA_ARG(
    emplace_return_type,emplace,emplace_impl,iterator,position)

  std::pair<iterator,bool> insert(iterator position,const value_type& x)
  {
    std::pair<final_node_type*,bool> p=this->final_insert_(x);
    if(p.second&&position.get_node()!=header()){
      relink(position.get_node(),p.first);
    }
    return std::pair<iterator,bool>(make_iterator(p.first),p.second);
  }

  std::pair<iterator,bool> insert(iterator position,value_type&& x)
  {
    std::pair<final_node_type*,bool> p=this->final_insert_rv_(x);
    if(p.second&&position.get_node()!=header()){
      relink(position.get_node(),p.first);
    }
    return std::pair<iterator,bool>(make_iterator(p.first),p.second);
  }

  void insert(iterator position,size_type n,value_param_type x)
  {
    for(size_type i=0;i<n;++i)insert(position,x);
  }

  template<typename InputIterator>
  void insert(iterator position,InputIterator first,InputIterator last)
  {
    insert_iter(position,first,last,brigand::not_<std::is_integral<InputIterator> >());
  }

  void insert(iterator position,std::initializer_list<value_type> list)
  {
    insert(position,list.begin(),list.end());
  }

  iterator erase(iterator position)
  {
    this->final_erase_(static_cast<final_node_type*>(position++.get_node()));
    return position;
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

  void swap(sequenced_index<SuperMeta,TagList>& x)
  {
    this->final_swap_(x.final());
  }

  void clear()noexcept
  {
    this->final_clear_();
  }

  /* list operations */

  void splice(iterator position,sequenced_index<SuperMeta,TagList>& x)
  {
    iterator first=x.begin(),last=x.end();
    while(first!=last){
      if(insert(position,*first).second)first=x.erase(first);
      else ++first;
    }
  }

  void splice(iterator position,sequenced_index<SuperMeta,TagList>& x,iterator i)
  {
    if(&x==this){
      if(position!=i)relink(position.get_node(),i.get_node());
    }
    else{
      if(insert(position,*i).second){
        x.erase(i);

      }
    }
  }

  void splice(
    iterator position,sequenced_index<SuperMeta,TagList>& x,
    iterator first,iterator last)
  {
    if(&x==this){
      if(position!=last)relink(
        position.get_node(),first.get_node(),last.get_node());
    }
    else{
      while(first!=last){
        if(insert(position,*first).second)first=x.erase(first);
        else ++first;
      }
    }
  }

  void remove(value_param_type value)
  {
    using namespace std::placeholders;
    sequenced_index_remove(
      *this,
      ::std::bind(MULTI_INDEX_EQUAL_TO<value_type>(),_1,value));
  }

  template<typename Predicate>
  void remove_if(Predicate pred)
  {
    sequenced_index_remove(*this,pred);
  }

  void unique()
  {
    sequenced_index_unique(*this,MULTI_INDEX_EQUAL_TO<value_type>());
  }

  template <class BinaryPredicate>
  void unique(BinaryPredicate binary_pred)
  {
    sequenced_index_unique(*this,binary_pred);
  }

  void merge(sequenced_index<SuperMeta,TagList>& x)
  {
    sequenced_index_merge(*this,x,MULTI_INDEX_LESS<value_type>());
  }

  template <typename Compare>
  void merge(sequenced_index<SuperMeta,TagList>& x,Compare comp)
  {
    sequenced_index_merge(*this,x,comp);
  }

  void sort()
  {
    sequenced_index_sort(header(),MULTI_INDEX_LESS<value_type>());
  }

  template <typename Compare>
  void sort(Compare comp)
  {
    sequenced_index_sort(header(),comp);
  }

  void reverse()noexcept
  {
    node_impl_type::reverse(header()->impl());
  }

  /* rearrange operations */

  void relocate(iterator position,iterator i)
  {
    if(position!=i)relink(position.get_node(),i.get_node());
  }

  void relocate(iterator position,iterator first,iterator last)
  {
    if(position!=last)relink(
      position.get_node(),first.get_node(),last.get_node());
  }

  template<typename InputIterator>
  void rearrange(InputIterator first)
  {
    node_type* pos=header();
    for(size_type s=size();s--;){
      const value_type& v=*first++;
      relink(pos,node_from_value<node_type>(&v));
    }
  }

protected:
  sequenced_index(const ctor_args_list& args_list,const allocator_type& al):
    super(args_list.get_tail(),al)
  {
    empty_initialize();
  }

  sequenced_index(const sequenced_index<SuperMeta,TagList>& x):
    super(x)
  {
    /* the actual copying takes place in subsequent call to copy_() */
  }

  sequenced_index(
    const sequenced_index<SuperMeta,TagList>& x,do_not_copy_elements_tag):
    super(x,do_not_copy_elements_tag())

  {
    empty_initialize();
  }

  ~sequenced_index()
  {
    /* the container is guaranteed to be empty by now */
  }

  iterator       make_iterator(node_type* node){return iterator(node);}
  const_iterator make_iterator(node_type* node)const
                   {return const_iterator(node);}

  void copy_(
    const sequenced_index<SuperMeta,TagList>& x,const copy_map_type& map)
  {
    node_type* org=x.header();
    node_type* cpy=header();
    do{
      node_type* next_org=node_type::from_impl(org->next());
      node_type* next_cpy=map.find(static_cast<final_node_type*>(next_org));
      cpy->next()=next_cpy->impl();
      next_cpy->prior()=cpy->impl();
      org=next_org;
      cpy=next_cpy;
    }while(org!=x.header());

    super::copy_(x,map);
  }

  template<typename Variant>
  final_node_type* insert_(
    value_param_type v,final_node_type*& x,Variant variant)
  {
    final_node_type* res=super::insert_(v,x,variant);
    if(res==x)link(static_cast<node_type*>(x));
    return res;
  }

  template<typename Variant>
  final_node_type* insert_(
    value_param_type v,node_type* position,final_node_type*& x,Variant variant)
  {
    final_node_type* res=super::insert_(v,position,x,variant);
    if(res==x)link(static_cast<node_type*>(x));
    return res;
  }

  void erase_(node_type* x)
  {
    unlink(x);
    super::erase_(x);
  }

  void delete_all_nodes_()
  {
    for(node_type* x=node_type::from_impl(header()->next());x!=header();){
      node_type* y=node_type::from_impl(x->next());
      this->final_delete_node_(static_cast<final_node_type*>(x));
      x=y;
    }
  }

  void clear_()
  {
    super::clear_();
    empty_initialize();
  }

  void swap_(sequenced_index<SuperMeta,TagList>& x)
  {
    super::swap_(x);
  }

  void swap_elements_(sequenced_index<SuperMeta,TagList>& x)
  {
    super::swap_elements_(x);
  }

  template<typename Variant>
  bool replace_(value_param_type v,node_type* x,Variant variant)
  {
    return super::replace_(v,x,variant);
  }

  bool modify_(node_type* x)
  {
    try {
      if(!super::modify_(x)){
        unlink(x);
        return false;
      }
      else return true;
    }
    catch(...){
      unlink(x);
      throw;
    }
  }

  bool modify_rollback_(node_type* x)
  {
    return super::modify_rollback_(x);
  }

private:
  node_type* header()const{return this->final_header();}

  void empty_initialize()
  {
    header()->prior()=header()->next()=header()->impl();
  }

  void link(node_type* x)
  {
    node_impl_type::link(x->impl(),header()->impl());
  };

  static void unlink(node_type* x)
  {
    node_impl_type::unlink(x->impl());
  }

  static void relink(node_type* position,node_type* x)
  {
    node_impl_type::relink(position->impl(),x->impl());
  }

  static void relink(node_type* position,node_type* first,node_type* last)
  {
    node_impl_type::relink(
      position->impl(),first->impl(),last->impl());
  }

  template <class InputIterator>
  void assign_iter(InputIterator first,InputIterator last,brigand::true_type)
  {
    clear();
    for(;first!=last;++first)this->final_insert_ref_(*first);
  }

  void assign_iter(size_type n,value_param_type value,brigand::false_type)
  {
    clear();
    for(size_type i=0;i<n;++i)push_back(value);
  }

  template<typename InputIterator>
  void insert_iter(
    iterator position,InputIterator first,InputIterator last,brigand::true_type)
  {
    for(;first!=last;++first){
      std::pair<final_node_type*,bool> p=
        this->final_insert_ref_(*first);
      if(p.second&&position.get_node()!=header()){
        relink(position.get_node(),p.first);
      }
    }
  }

  void insert_iter(
    iterator position,size_type n,value_param_type x,brigand::false_type)
  {
    for(size_type i=0;i<n;++i)insert(position,x);
  }

  template<typename... Args>
  std::pair<iterator,bool> emplace_front_impl(Args&&... args)
  {
    return emplace_impl(begin(),std::forward<Args>(args)...);
  }

  template<typename... Args>
  std::pair<iterator,bool> emplace_back_impl(Args&&... args)
  {
    return emplace_impl(end(),std::forward<Args>(args)...);
  }

  template<typename... Args>
  std::pair<iterator,bool> emplace_impl(iterator position,Args&&... args)
  {
    std::pair<final_node_type*,bool> p=
      this->final_emplace_(std::forward<Args>(args)...);
    if(p.second&&position.get_node()!=header()){
      relink(position.get_node(),p.first);
    }
    return std::pair<iterator,bool>(make_iterator(p.first),p.second);
  }
};

/* comparison */

template<
  typename SuperMeta1,typename TagList1,
  typename SuperMeta2,typename TagList2
>
bool operator==(
  const sequenced_index<SuperMeta1,TagList1>& x,
  const sequenced_index<SuperMeta2,TagList2>& y)
{
  return x.size()==y.size()&&std::equal(x.begin(),x.end(),y.begin());
}

template<
  typename SuperMeta1,typename TagList1,
  typename SuperMeta2,typename TagList2
>
bool operator<(
  const sequenced_index<SuperMeta1,TagList1>& x,
  const sequenced_index<SuperMeta2,TagList2>& y)
{
  return std::lexicographical_compare(x.begin(),x.end(),y.begin(),y.end());
}

template<
  typename SuperMeta1,typename TagList1,
  typename SuperMeta2,typename TagList2
>
bool operator!=(
  const sequenced_index<SuperMeta1,TagList1>& x,
  const sequenced_index<SuperMeta2,TagList2>& y)
{
  return !(x==y);
}

template<
  typename SuperMeta1,typename TagList1,
  typename SuperMeta2,typename TagList2
>
bool operator>(
  const sequenced_index<SuperMeta1,TagList1>& x,
  const sequenced_index<SuperMeta2,TagList2>& y)
{
  return y<x;
}

template<
  typename SuperMeta1,typename TagList1,
  typename SuperMeta2,typename TagList2
>
bool operator>=(
  const sequenced_index<SuperMeta1,TagList1>& x,
  const sequenced_index<SuperMeta2,TagList2>& y)
{
  return !(x<y);
}

template<
  typename SuperMeta1,typename TagList1,
  typename SuperMeta2,typename TagList2
>
bool operator<=(
  const sequenced_index<SuperMeta1,TagList1>& x,
  const sequenced_index<SuperMeta2,TagList2>& y)
{
  return !(x>y);
}

/*  specialized algorithms */

template<typename SuperMeta,typename TagList>
void swap(
  sequenced_index<SuperMeta,TagList>& x,
  sequenced_index<SuperMeta,TagList>& y)
{
  x.swap(y);
}

} /* namespace multi_index::detail */

/* sequenced index specifier */

template <typename TagList>
struct sequenced
{
  static_assert(detail::is_tag<TagList>::value, "");

  template<typename Super>
  struct node_class
  {
    typedef detail::sequenced_index_node<Super> type;
  };

  template<typename SuperMeta>
  struct index_class
  {
    typedef detail::sequenced_index<SuperMeta,typename TagList::type> type;
  };
};

} /* namespace multi_index */
