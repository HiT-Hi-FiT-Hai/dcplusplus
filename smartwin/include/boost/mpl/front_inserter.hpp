
#ifndef BOOST_MPL_FRONT_INSERTER_HPP_INCLUDED
#define BOOST_MPL_FRONT_INSERTER_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2003-2004
// Copyright David Abrahams 2003-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source: /cvsroot/smartwin/SmartWin/include/boost/mpl/front_inserter.hpp,v $
// $Date: 2006/03/24 17:33:48 $
// $Revision: 1.3 $

#include <boost/mpl/push_front.hpp>
#include <boost/mpl/inserter.hpp>

namespace boost { namespace mpl {

template<
      typename Sequence
    >
struct front_inserter
    : inserter< Sequence,push_front<> >
{
};

}}

#endif // BOOST_MPL_FRONT_INSERTER_HPP_INCLUDED