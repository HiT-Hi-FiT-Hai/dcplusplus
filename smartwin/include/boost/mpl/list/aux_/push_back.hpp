
#ifndef BOOST_MPL_LIST_AUX_PUSH_BACK_HPP_INCLUDED
#define BOOST_MPL_LIST_AUX_PUSH_BACK_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source: /cvsroot/smartwin/SmartWin/include/boost/mpl/list/aux_/push_back.hpp,v $
// $Date: 2006/03/24 17:34:04 $
// $Revision: 1.3 $

#include <boost/mpl/push_back_fwd.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/list/aux_/tag.hpp>

namespace boost { namespace mpl {

template< typename Tag > struct has_push_back_impl;

template<>
struct has_push_back_impl< aux::list_tag >
{
    template< typename Seq > struct apply
        : false_
    {
    };
};

}}

#endif // BOOST_MPL_LIST_AUX_PUSH_BACK_HPP_INCLUDED
