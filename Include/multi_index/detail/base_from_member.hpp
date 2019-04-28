//  boost utility/base_from_member.hpp header file  --------------------------//

//  Copyright 2001, 2003, 2004, 2012 Daryle Walker.  Use, modification, and
//  distribution are subject to the Boost Software License, Version 1.0.  (See
//  accompanying file LICENSE_1_0.txt or a copy at
//  <http://www.boost.org/LICENSE_1_0.txt>.)

//  See <http://www.boost.org/libs/utility/> for the library's home page.

#pragma once

#include <type_traits>

//  Base-from-member arity configuration macro  ------------------------------//

// The following macro determines how many arguments will be in the largest
// constructor template of base_from_member.  Constructor templates will be
// generated from one argument to this maximum.  Code from other files can read
// this number if they need to always match the exact maximum base_from_member
// uses.  The maximum constructor length can be changed by overriding the
// #defined constant.  Make sure to apply the override, if any, for all source
// files during project compiling for consistency.

// Contributed by Jonathan Turkanis

#pragma once


namespace multi_index
{
namespace detail
{

//  Type-unmarking class template  -------------------------------------------//

// Type-trait to get the raw type, i.e. the type without top-level reference nor
// cv-qualification, from a type expression.  Mainly for function arguments, any
// reference part is stripped first.

// Contributed by Daryle Walker

template < typename T >
struct remove_cv_ref
{
    typedef typename std::remove_cv<typename
     std::remove_reference<T>::type>::type  type;

};  // detail::remove_cv_ref

//  Unmarked-type comparison class template  ---------------------------------//

// Type-trait to check if two type expressions have the same raw type.

// Contributed by Daryle Walker, based on a work-around by Luc Danton

template < typename T, typename U >
struct is_related
    : public std::is_same<
     typename ::multi_index::detail::remove_cv_ref<T>::type,
     typename ::multi_index::detail::remove_cv_ref<U>::type >
{};

//  Enable-if-on-unidentical-unmarked-type class template  -------------------//

// Enable-if on the first two type expressions NOT having the same raw type.

// Contributed by Daryle Walker, based on a work-around by Luc Danton

template<typename ...T>
struct enable_if_unrelated
    : public std::enable_if<true>
{};

template<typename T, typename U, typename ...U2>
struct enable_if_unrelated<T, U, U2...>
    : public std::enable_if< !::multi_index::detail::is_related<T, U>::value >
{};

}  // namespace multi_index::detail


//  Base-from-member class template  -----------------------------------------//

// Helper to initialize a base object so a derived class can use this
// object in the initialization of another base class.  Used by
// Dietmar Kuehl from ideas by Ron Klatcho to solve the problem of a
// base class needing to be initialized by a member.

// Contributed by Daryle Walker

template < typename MemberType, int UniqueID = 0 >
class base_from_member
{
protected:
    MemberType  member;

    template <typename ...T, typename EnableIf = typename
     ::multi_index::detail::enable_if_unrelated<base_from_member, T...>::type>
    explicit constexpr base_from_member( T&& ...x ): member( static_cast<T&&>(x)... )
    {}

};  // multi_index::base_from_member

template < typename MemberType, int UniqueID >
class base_from_member<MemberType&, UniqueID>
{
protected:
    MemberType& member;

    explicit constexpr base_from_member( MemberType& x ) noexcept :
        member( x )
    {}

};  // multi_index::base_from_member

}  // namespace multi_index
