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
#include <multi_index/detail/do_not_copy_elements_tag.hpp>
#include <multi_index/detail/index_node_base.hpp>
#include <multi_index/detail/rnd_node_iterator.hpp>
#include <multi_index/detail/rnd_index_node.hpp>
#include <multi_index/detail/rnd_index_ops.hpp>
#include <multi_index/detail/rnd_index_ptr_array.hpp>
#include <multi_index/detail/scope_guard.hpp>
#include <multi_index/detail/vartempl_support.hpp>
#include <multi_index/config.hpp>
#include <multi_index/random_access_index_fwd.hpp>
#include <multi_index/tuple.hpp>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace multi_index{

namespace detail{

/* random_access_index adds a layer of random access indexing
 * to a given Super
 */

template<typename SuperMeta,typename TagList>
class random_access_index:
  protected SuperMeta::type

{
  typedef typename SuperMeta::type                 super;

protected:
  typedef random_access_index_node<
    typename super::node_type>                     node_type;

private:
  typedef typename node_type::impl_type            node_impl_type;
  typedef random_access_index_ptr_array<
    typename super::final_allocator_type>          ptr_array;
  typedef typename ptr_array::pointer              node_impl_ptr_pointer;

public:
  /* types */

  typedef typename node_type::value_type           value_type;
  typedef tuples::null_type                        ctor_args;
  typedef typename super::final_allocator_type     allocator_type;
  typedef typename std::allocator_traits<
    allocator_type>::value_type&                   reference;
  typedef typename std::allocator_traits<
    allocator_type>::value_type const&             const_reference;
  typedef rnd_node_iterator<node_type>             iterator;
  typedef iterator                                 const_iterator;

  typedef std::size_t                              size_type;
  typedef std::ptrdiff_t                           difference_type;
  typedef typename std::allocator_traits<
    allocator_type>::pointer                       pointer;
  typedef typename std::allocator_traits<
    allocator_type>::const_pointer                 const_pointer;
  typedef typename
    std::reverse_iterator<iterator>              reverse_iterator;
  typedef typename
    std::reverse_iterator<const_iterator>        const_reverse_iterator;
  typedef TagList                                  tag_list;

protected:
  typedef typename super::final_node_type     final_node_type;
  typedef tuples::cons<
    ctor_args,
    typename super::ctor_args_list>           ctor_args_list;
  typedef brigand::push_front<
    typename super::index_type_list,
    random_access_index>                      index_type_list;
  typedef brigand::push_front<
    typename super::iterator_type_list,
    iterator>                                 iterator_type_list;
  typedef brigand::push_front<
    typename super::const_iterator_type_list,
    const_iterator>                           const_iterator_type_list;
  typedef typename super::copy_map_type       copy_map_type;

private:
  typedef typename call_traits<
    value_type>::param_type                   value_param_type;

  /* Needed to avoid commas in MULTI_INDEX_OVERLOADS_TO_VARTEMPL
   * expansion.
   */

  typedef std::pair<iterator,bool>            emplace_return_type;

public:

  /* construct/copy/destroy
   * Default and copy ctors are in the protected section as indices are
   * not supposed to be created on their own. No range ctor either.
   */

  random_access_index<SuperMeta,TagList>& operator=(
    const random_access_index<SuperMeta,TagList>& x)
  {
    this->final()=x.final();
    return *this;
  }

  random_access_index<SuperMeta,TagList>& operator=(
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

  iterator begin()noexcept
    {return make_iterator(node_type::from_impl(*ptrs.begin()));}
  const_iterator begin()const noexcept
    {return make_iterator(node_type::from_impl(*ptrs.begin()));}
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
  size_type capacity()const noexcept{return ptrs.capacity();}

  void reserve(size_type n)
  {
    ptrs.reserve(n);
  }

  void shrink_to_fit()
  {
    ptrs.shrink_to_fit();
  }

  void resize(size_type n)
  {
    if(n>size())
      for(size_type m=n-size();m--;)
        this->final_emplace_();
    else if(n<size())erase(begin()+n,end());
  }

  void resize(size_type n,value_param_type x)
  {
    if(n>size())for(size_type m=n-size();m--;)this->final_insert_(x);
    else if(n<size())erase(begin()+n,end());
  }

  /* access: no non-const versions provided as random_access_index
   * handles const elements.
   */

  const_reference operator[](size_type n)const
  {
    return node_type::from_impl(*ptrs.at(n))->value();
  }

  const_reference at(size_type n)const
  {
    if(n>=size())
      throw std::out_of_range("random access index");
    return node_type::from_impl(*ptrs.at(n))->value();
  }

  const_reference front()const{return operator[](0);}
  const_reference back()const{return operator[](size()-1);}

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
      relocate(position.get_node(),p.first);
    }
    return std::pair<iterator,bool>(make_iterator(p.first),p.second);
  }

  std::pair<iterator,bool> insert(iterator position,value_type&& x)
  {
    std::pair<final_node_type*,bool> p=this->final_insert_rv_(x);
    if(p.second&&position.get_node()!=header()){
      relocate(position.get_node(),p.first);
    }
    return std::pair<iterator,bool>(make_iterator(p.first),p.second);
  }

  void insert(iterator position,size_type n,value_param_type x)
  {
    size_type s=0;
    try {
      while(n--){
        if(push_back(x).second)++s;
      }
    }
    catch(...){
      relocate(position,end()-s,end());
      throw;
    }
    relocate(position,end()-s,end());
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
    difference_type n=last-first;
    relocate(end(),first,last);
    while(n--)pop_back();
    return last;
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

  void swap(random_access_index<SuperMeta,TagList>& x)
  {
    this->final_swap_(x.final());
  }

  void clear()noexcept
  {
    this->final_clear_();
  }

  /* list operations */

  void splice(iterator position,random_access_index<SuperMeta,TagList>& x)
  {
    iterator  first=x.begin(),last=x.end();
    size_type n=0;
    try {
      while(first!=last){
        if(push_back(*first).second){
          first=x.erase(first);
          ++n;
        }
        else ++first;
      }
    }
    catch(...){
      relocate(position,end()-n,end());
      throw;
    }
    relocate(position,end()-n,end());
  }

  void splice(
    iterator position,random_access_index<SuperMeta,TagList>& x,iterator i)
  {
    if(&x==this)relocate(position,i);
    else{
      if(insert(position,*i).second){
        x.erase(i);

      }
    }
  }

  void splice(
    iterator position,random_access_index<SuperMeta,TagList>& x,
    iterator first,iterator last)
  {
    if(&x==this)relocate(position,first,last);
    else{
      size_type n=0;
      try {
        while(first!=last){
          if(push_back(*first).second){
            first=x.erase(first);
            ++n;
          }
          else ++first;
        }
      }
      catch(...){
        relocate(position,end()-n,end());
        throw;
      }
      relocate(position,end()-n,end());
    }
  }

  void remove(value_param_type value)
  {
    using namespace std::placeholders;
    difference_type n=
      end()-make_iterator(
        random_access_index_remove<node_type>(
          ptrs,
          ::std::bind(MULTI_INDEX_EQUAL_TO<value_type>(),_1,value)));
    while(n--)pop_back();
  }

  template<typename Predicate>
  void remove_if(Predicate pred)
  {
    difference_type n=
      end()-make_iterator(random_access_index_remove<node_type>(ptrs,pred));
    while(n--)pop_back();
  }

  void unique()
  {
    difference_type n=
      end()-make_iterator(
        random_access_index_unique<node_type>(
          ptrs,MULTI_INDEX_EQUAL_TO<value_type>()));
    while(n--)pop_back();
  }

  template <class BinaryPredicate>
  void unique(BinaryPredicate binary_pred)
  {
    difference_type n=
      end()-make_iterator(
        random_access_index_unique<node_type>(ptrs,binary_pred));
    while(n--)pop_back();
  }

  void merge(random_access_index<SuperMeta,TagList>& x)
  {
    if(this!=&x){
      size_type s=size();
      splice(end(),x);
      random_access_index_inplace_merge<node_type>(
        get_allocator(),ptrs,ptrs.at(s),MULTI_INDEX_LESS<value_type>());
    }
  }

  template <typename Compare>
  void merge(random_access_index<SuperMeta,TagList>& x,Compare comp)
  {
    if(this!=&x){
      size_type s=size();
      splice(end(),x);
      random_access_index_inplace_merge<node_type>(
        get_allocator(),ptrs,ptrs.at(s),comp);
    }
  }

  void sort()
  {
    random_access_index_sort<node_type>(
      get_allocator(),ptrs,MULTI_INDEX_LESS<value_type>());
  }

  template <typename Compare>
  void sort(Compare comp)
  {
    random_access_index_sort<node_type>(
      get_allocator(),ptrs,comp);
  }

  void reverse()noexcept
  {
    node_impl_type::reverse(ptrs.begin(),ptrs.end());
  }

  /* rearrange operations */

  void relocate(iterator position,iterator i)
  {
    if(position!=i)relocate(position.get_node(),i.get_node());
  }

  void relocate(iterator position,iterator first,iterator last)
  {
    if(position!=last)relocate(
      position.get_node(),first.get_node(),last.get_node());
  }

  template<typename InputIterator>
  void rearrange(InputIterator first)
  {
    for(node_impl_ptr_pointer p0=ptrs.begin(),p0_end=ptrs.end();
        p0!=p0_end;++first,++p0){
      const value_type& v1=*first;
      node_impl_ptr_pointer p1=node_from_value<node_type>(&v1)->up();

      std::swap(*p0,*p1);
      (*p0)->up()=p0;
      (*p1)->up()=p1;
    }
  }

protected:
  random_access_index(
    const ctor_args_list& args_list,const allocator_type& al):
    super(args_list.get_tail(),al),
    ptrs(al,header()->impl(),0)
  {
  }

  random_access_index(const random_access_index<SuperMeta,TagList>& x):
    super(x),

    ptrs(x.get_allocator(),header()->impl(),x.size())
  {
    /* The actual copying takes place in subsequent call to copy_().
     */
  }

  random_access_index(
    const random_access_index<SuperMeta,TagList>& x,do_not_copy_elements_tag):
    super(x,do_not_copy_elements_tag()),

    ptrs(x.get_allocator(),header()->impl(),0)
  {
  }

  ~random_access_index()
  {
    /* the container is guaranteed to be empty by now */
  }

  iterator       make_iterator(node_type* node){return iterator(node);}
  const_iterator make_iterator(node_type* node)const
                   {return const_iterator(node);}

  void copy_(
    const random_access_index<SuperMeta,TagList>& x,const copy_map_type& map)
  {
    for(node_impl_ptr_pointer begin_org=x.ptrs.begin(),
                              begin_cpy=ptrs.begin(),
                              end_org=x.ptrs.end();
        begin_org!=end_org;++begin_org,++begin_cpy){
      *begin_cpy=
         static_cast<node_type*>(
           map.find(
             static_cast<final_node_type*>(
               node_type::from_impl(*begin_org))))->impl();
      (*begin_cpy)->up()=begin_cpy;
    }

    super::copy_(x,map);
  }

  template<typename Variant>
  final_node_type* insert_(
    value_param_type v,final_node_type*& x,Variant variant)
  {
    ptrs.room_for_one();
    final_node_type* res=super::insert_(v,x,variant);
    if(res==x)ptrs.push_back(static_cast<node_type*>(x)->impl());
    return res;
  }

  template<typename Variant>
  final_node_type* insert_(
    value_param_type v,node_type* position,final_node_type*& x,Variant variant)
  {
    ptrs.room_for_one();
    final_node_type* res=super::insert_(v,position,x,variant);
    if(res==x)ptrs.push_back(static_cast<node_type*>(x)->impl());
    return res;
  }

  void erase_(node_type* x)
  {
    ptrs.erase(x->impl());
    super::erase_(x);
  }

  void delete_all_nodes_()
  {
    for(node_impl_ptr_pointer x=ptrs.begin(),x_end=ptrs.end();x!=x_end;++x){
      this->final_delete_node_(
        static_cast<final_node_type*>(node_type::from_impl(*x)));
    }
  }

  void clear_()
  {
    super::clear_();
    ptrs.clear();
  }

  void swap_(random_access_index<SuperMeta,TagList>& x)
  {
    ptrs.swap(x.ptrs);
    super::swap_(x);
  }

  void swap_elements_(random_access_index<SuperMeta,TagList>& x)
  {
    ptrs.swap(x.ptrs);
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
        ptrs.erase(x->impl());
        return false;
      }
      else return true;
    }
    catch(...){
      ptrs.erase(x->impl());
      throw;
    }
  }

  bool modify_rollback_(node_type* x)
  {
    return super::modify_rollback_(x);
  }

private:
  node_type* header()const{return this->final_header();}

  static void relocate(node_type* position,node_type* x)
  {
    node_impl_type::relocate(position->up(),x->up());
  }

  static void relocate(node_type* position,node_type* first,node_type* last)
  {
    node_impl_type::relocate(
      position->up(),first->up(),last->up());
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
    size_type s=0;
    try {
      for(;first!=last;++first){
        if(this->final_insert_ref_(*first).second)++s;
      }
    }
    catch(...){
      relocate(position,end()-s,end());
      throw;
    }
    relocate(position,end()-s,end());
  }

  void insert_iter(
    iterator position,size_type n,value_param_type x,brigand::false_type)
  {
    size_type  s=0;
    try {
      while(n--){
        if(push_back(x).second)++s;
      }
    }
    catch(...){
      relocate(position,end()-s,end());
      throw;
    }
    relocate(position,end()-s,end());
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
      relocate(position.get_node(),p.first);
    }
    return std::pair<iterator,bool>(make_iterator(p.first),p.second);
  }

  ptr_array ptrs;
};

/* comparison */

template<
  typename SuperMeta1,typename TagList1,
  typename SuperMeta2,typename TagList2
>
bool operator==(
  const random_access_index<SuperMeta1,TagList1>& x,
  const random_access_index<SuperMeta2,TagList2>& y)
{
  return x.size()==y.size()&&std::equal(x.begin(),x.end(),y.begin());
}

template<
  typename SuperMeta1,typename TagList1,
  typename SuperMeta2,typename TagList2
>
bool operator<(
  const random_access_index<SuperMeta1,TagList1>& x,
  const random_access_index<SuperMeta2,TagList2>& y)
{
  return std::lexicographical_compare(x.begin(),x.end(),y.begin(),y.end());
}

template<
  typename SuperMeta1,typename TagList1,
  typename SuperMeta2,typename TagList2
>
bool operator!=(
  const random_access_index<SuperMeta1,TagList1>& x,
  const random_access_index<SuperMeta2,TagList2>& y)
{
  return !(x==y);
}

template<
  typename SuperMeta1,typename TagList1,
  typename SuperMeta2,typename TagList2
>
bool operator>(
  const random_access_index<SuperMeta1,TagList1>& x,
  const random_access_index<SuperMeta2,TagList2>& y)
{
  return y<x;
}

template<
  typename SuperMeta1,typename TagList1,
  typename SuperMeta2,typename TagList2
>
bool operator>=(
  const random_access_index<SuperMeta1,TagList1>& x,
  const random_access_index<SuperMeta2,TagList2>& y)
{
  return !(x<y);
}

template<
  typename SuperMeta1,typename TagList1,
  typename SuperMeta2,typename TagList2
>
bool operator<=(
  const random_access_index<SuperMeta1,TagList1>& x,
  const random_access_index<SuperMeta2,TagList2>& y)
{
  return !(x>y);
}

/*  specialized algorithms */

template<typename SuperMeta,typename TagList>
void swap(
  random_access_index<SuperMeta,TagList>& x,
  random_access_index<SuperMeta,TagList>& y)
{
  x.swap(y);
}

} /* namespace multi_index::detail */

/* random access index specifier */

template <typename TagList>
struct random_access
{
  static_assert(detail::is_tag<TagList>::value, "");

  template<typename Super>
  struct node_class
  {
    typedef detail::random_access_index_node<Super> type;
  };

  template<typename SuperMeta>
  struct index_class
  {
    typedef detail::random_access_index<
      SuperMeta,typename TagList::type>  type;
  };
};

} /* namespace multi_index */
