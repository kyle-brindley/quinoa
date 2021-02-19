// *****************************************************************************
/*!
  \file      src/DiffEq/WrightFisher/ConfigureWrightFisher.hpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019-2021 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Register and compile configuration on the Wright-Fisher SDE
  \details   Register and compile configuration on the Wright-Fisher SDE.
*/
// *****************************************************************************
#ifndef ConfigureWrightFisher_h
#define ConfigureWrightFisher_h

#include <set>
#include <map>
#include <string>
#include <utility>

#include "DiffEqFactory.hpp"
#include "Walker/Options/DiffEq.hpp"

namespace walker {

//! Register Wright-Fisher SDE into DiffEq factory
void registerWrightFisher( DiffEqFactory& f, std::set< ctr::DiffEqType >& t );

//! Return information on the Wright-Fisher SDE
std::vector< std::pair< std::string, std::string > >
infoWrightFisher( std::map< ctr::DiffEqType, tk::ctr::ncomp_t >& cnt );

} // walker::

#endif // ConfigureWrightFisher_h
