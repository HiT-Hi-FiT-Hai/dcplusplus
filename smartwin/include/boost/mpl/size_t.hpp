
#ifndef BOOST_MPL_SIZE_T_HPP_INCLUDED
#define BOOST_MPL_SIZE_T_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Source: /cvsroot/smartwin/SmartWin/include/boost/mpl/size_t.hpp,v $
// $Date: 2006/03/24 17:33:48 $
// $Revision: 1.3 $

#include <boost/mpl/size_t_fwd.hpp>

#define AUX_WRAPPER_VALUE_TYPE std::size_t
#define AUX_WRAPPER_NAME size_t
#define AUX_WRAPPER_PARAMS(N) std::size_t N

#include <boost/mpl/aux_/integral_wrapper.hpp>

#endif // BOOST_MPL_SIZE_T_HPP_INCLUDED
