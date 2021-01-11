// *****************************************************************************
/*!
  \file      src/Inciter/ConjugateGradients.cpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019-2020 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Charm++ chare array for distributed conjugate gradients.
  \details   Charm++ chare array for asynchronous distributed
    conjugate gradients linear solver.
*/
// *****************************************************************************

#include "ConjugateGradients.hpp"

using tk::ConjugateGradients;

ConjugateGradients::ConjugateGradients()
// *****************************************************************************
//  Constructor
// *****************************************************************************
{
}

#include "NoWarning/conjugategradients.def.h"
