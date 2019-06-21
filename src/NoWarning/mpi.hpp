// *****************************************************************************
/*!
  \file      src/NoWarning/mpi.hpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Include mpi.h with turning off specific compiler warnings
*/
// *****************************************************************************
#ifndef nowarning_mpi_h
#define nowarning_mpi_h

#include "Macro.hpp"

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wold-style-cast"
  #pragma clang diagnostic ignored "-Wsign-conversion"
  #pragma clang diagnostic ignored "-Wdeprecated"
  #pragma clang diagnostic ignored "-Wcast-align"
  #pragma clang diagnostic ignored "-Wextra-semi"
  #pragma clang diagnostic ignored "-Wundef"
  #pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#elif defined(STRICT_GNUC)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wlong-long"
  #pragma GCC diagnostic ignored "-Wcast-qual"
  #pragma GCC diagnostic ignored "-Wredundant-decls"
#endif

#include <mpi.h>

#if defined(__clang__)
  #pragma clang diagnostic pop
#elif defined(STRICT_GNUC)
  #pragma GCC diagnostic pop
#endif


#endif // nowarning_mpi_h
