// *****************************************************************************
/*!
  \file      src/NoWarning/conjugategradients.def.h
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019-2020 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Include conjugategradients.def.h with turning off specific
             compiler warnings
*/
// *****************************************************************************
#ifndef nowarning_conjugategradients_def_h
#define nowarning_conjugategradients_def_h

#include "Macro.hpp"

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wold-style-cast"
  #pragma clang diagnostic ignored "-Wsign-conversion"
  #pragma clang diagnostic ignored "-Wshorten-64-to-32"
  #pragma clang diagnostic ignored "-Wunused-variable"
  #pragma clang diagnostic ignored "-Wcast-qual"
#elif defined(STRICT_GNUC)
  #pragma GCC diagnostic push
//  #pragma GCC diagnostic ignored "-Wcast-qual"
#endif

#include "../LinearSolver/conjugategradients.def.h"

#if defined(__clang__)
  #pragma clang diagnostic pop
#elif defined(STRICT_GNUC)
  #pragma GCC diagnostic pop
#endif

#endif // nowarning_conjugategradients_def_h
