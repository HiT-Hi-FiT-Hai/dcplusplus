
#ifndef BOOST_MPL_BACK_HPP_INCLUDED
#define BOOST_MPL_BACK_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source: /cvsroot/smartwin/SmartWin/include/boost/mpl/back.hpp,v $
// $Date: 2006/03/24 17:33:48 $
// $Revision: 1.3 $

#include <boost/mpl/back_fwd.hpp>
#include <boost/mpl/aux_/back_impl.hpp>
#include <boost/mpl/sequence_tag.hpp>
#include <boost/mpl/aux_/na_spec.hpp>
#include <boost/mpl/aux_/lambda_support.hpp>

namespace boost { namespace mpl {

template<
      typename BOOST_MPL_AUX_NA_PARAM(Sequence)
    >
struct back
    : back_impl< typename sequence_tag<Sequence>::type >
        ::template apply< Sequence >
{
    BOOST_MPL_AUX_LAMBDA_SUPPORT(1,back,(Sequence))
};

BOOST_MPL_AUX_NA_SPEC(1, back)

}}

#endif // BOOST_MPL_BACK_HPP_INCLUDED