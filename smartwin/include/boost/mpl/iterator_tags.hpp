
#ifndef BOOST_MPL_ITERATOR_TAG_HPP_INCLUDED
#define BOOST_MPL_ITERATOR_TAG_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source: /cvsroot/smartwin/SmartWin/include/boost/mpl/iterator_tags.hpp,v $
// $Date: 2006/03/24 17:33:48 $
// $Revision: 1.3 $

#include <boost/mpl/int.hpp>

namespace boost { namespace mpl {

struct forward_iterator_tag       : int_<0> { typedef forward_iterator_tag type; };
struct bidirectional_iterator_tag : int_<1> { typedef bidirectional_iterator_tag type; };
struct random_access_iterator_tag : int_<2> { typedef random_access_iterator_tag type; };

}}

#endif // BOOST_MPL_ITERATOR_TAG_HPP_INCLUDED
