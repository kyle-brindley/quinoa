// *****************************************************************************
/*!
  \file      src/NoWarning/cartesian_product.h
  \author    J. Bakosi
  \date      Wed 11 May 2016 06:58:15 AM MDT
  \copyright 2012-2015, Jozsef Bakosi, 2016, Los Alamos National Security, LLC.
  \brief     Include cartesian_product.h with turning off specific compiler
             warnings
*/
// *****************************************************************************
#ifndef nowarning_cartesian_product_h
#define nowarning_cartesian_product_h

#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wold-style-cast"
  #pragma clang diagnostic ignored "-Wreserved-id-macro"
  #pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#elif defined(__GNUC__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Weffc++"
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

#include <boost/mpl/cartesian_product.hpp>

#if defined(__clang__)
  #pragma clang diagnostic pop
#elif defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif

#endif // nowarning_cartesian_product_h
