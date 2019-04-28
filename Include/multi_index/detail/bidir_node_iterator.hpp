/* Copyright 2003-2014 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 * See http://www.boost.org/libs/multi_index for library home page.
 */

#pragma once

#include <iterator>

namespace multi_index{

namespace detail{

/* Iterator class for node-based indices with bidirectional
 * iterators (ordered and sequenced indices.)
 */

template<typename Node>
class bidir_node_iterator:
  public std::iterator<std::bidirectional_iterator_tag,
    typename Node::value_type,
    std::ptrdiff_t,
    const typename Node::value_type*,
    const typename Node::value_type&>
{
public:
  /* coverity[uninit_ctor]: suppress warning */
  bidir_node_iterator(){}
  explicit bidir_node_iterator(Node* node_):node(node_){}

  const typename Node::value_type& operator*()const
  {
    return node->value();
  }

  const typename Node::value_type* operator->()const
  {
    return &node->value();
  }

  bidir_node_iterator& operator++()
  {
    Node::increment(node);
    return *this;
  }

  bidir_node_iterator operator++(int)
  {
    bidir_node_iterator copy(*this);
    operator++();
    return copy;
  }

  bidir_node_iterator& operator--()
  {
    Node::decrement(node);
    return *this;
  }

  bidir_node_iterator operator--(int)
  {
    bidir_node_iterator copy(*this);
    operator--();
    return copy;
  }

  /* get_node is not to be used by the user */

  typedef Node node_type;

  Node* get_node()const{return node;}

private:
  Node* node;
};

template<typename Node>
bool operator==(
  const bidir_node_iterator<Node>& x,
  const bidir_node_iterator<Node>& y)
{
  return x.get_node()==y.get_node();
}


template<typename Node>
bool operator!=(
  const bidir_node_iterator<Node>& x,
  const bidir_node_iterator<Node>& y)
{
  return !(x == y);
}

} /* namespace multi_index::detail */

} /* namespace multi_index */
