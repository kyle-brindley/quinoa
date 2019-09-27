// *****************************************************************************
/*!
  \file      src/PDE/Reconstruction.cpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Reconstruction for reconstructed discontinuous Galerkin methods
  \details   This file contains functions that reconstruct an "n"th order
    polynomial to an "n+1"th order polynomial using a least-squares
    reconstruction procedure.
*/
// *****************************************************************************

#include <array>
#include <vector>

#include "Vector.hpp"
#include "Reconstruction.hpp"

void
tk::lhsLeastSq_P0P1( const inciter::FaceData& fd,
  const Fields& geoElem,
  const Fields& geoFace,
  std::vector< std::array< std::array< real, 3 >, 3 > >& lhs_ls )
// *****************************************************************************
//  Compute lhs matrix for the least-squares reconstruction
//! \param[in] fd Face connectivity and boundary conditions object
//! \param[in] geoElem Element geometry array
//! \param[in] geoFace Face geometry array
//! \param[in,out] lhs_ls LHS reconstruction matrix
// *****************************************************************************
{
  const auto& esuf = fd.Esuf();

  // Compute internal and boundary face contributions
  for (auto f=0; f<esuf.size()/2; ++f)
  {
    Assert( esuf[2*f] > -1, "Left-side element detected as -1" );

    auto el = static_cast< std::size_t >(esuf[2*f]);
    auto er = esuf[2*f+1];

    std::array< real, 3 > geoElemR;
    std::size_t eR;

    // get a 3x3 system by applying the normal equation approach to the
    // least-squares overdetermined system

    if (er > -1)
    // internal face contribution
    {
      eR = static_cast< std::size_t >(er);
      geoElemR = {{ geoElem(eR,1,0), geoElem(eR,2,0), geoElem(eR,3,0) }};
    }
    else
    // boundary face contribution
    {
      geoElemR = {{ geoFace(f,4,0), geoFace(f,5,0), geoFace(f,6,0) }};
    }

    std::array< real, 3 > wdeltax{{ geoElemR[0]-geoElem(el,1,0),
                                    geoElemR[1]-geoElem(el,2,0),
                                    geoElemR[2]-geoElem(el,3,0) }};

    for (std::size_t idir=0; idir<3; ++idir)
    {
      // lhs matrix
      for (std::size_t jdir=0; jdir<3; ++jdir)
      {
        lhs_ls[el][idir][jdir] += wdeltax[idir] * wdeltax[jdir];
        if (er > -1) lhs_ls[eR][idir][jdir] += wdeltax[idir] * wdeltax[jdir];
      }
    }
  }
}

void
tk::intLeastSq_P0P1( ncomp_t ncomp,
                     ncomp_t offset,
                     const std::size_t rdof,
                     const inciter::FaceData& fd,
                     const Fields& geoElem,
                     const Fields& W,
                     std::vector< std::vector< std::array< real, 3 > > >& rhs_ls )
// *****************************************************************************
//  Compute internal surface contributions to the least-squares reconstruction
//! \param[in] ncomp Number of scalar components in this PDE system
//! \param[in] offset Offset this PDE system operates from
//! \param[in] rdof Maximum number of reconstructed degrees of freedom
//! \param[in] fd Face connectivity and boundary conditions object
//! \param[in] geoElem Element geometry array
//! \param[in] W Solution vector to be reconstructed at recent time step
//! \param[in,out] rhs_ls RHS reconstruction vector
// *****************************************************************************
{
  const auto& esuf = fd.Esuf();

  // Compute internal face contributions
  for (auto f=fd.Nbfac(); f<esuf.size()/2; ++f)
  {
    Assert( esuf[2*f] > -1 && esuf[2*f+1] > -1, "Interior element detected "
            "as -1" );

    auto el = static_cast< std::size_t >(esuf[2*f]);
    auto er = static_cast< std::size_t >(esuf[2*f+1]);

    // get a 3x3 system by applying the normal equation approach to the
    // least-squares overdetermined system
    std::array< real, 3 > wdeltax{{ geoElem(er,1,0)-geoElem(el,1,0),
                                    geoElem(er,2,0)-geoElem(el,2,0),
                                    geoElem(er,3,0)-geoElem(el,3,0) }};

    for (std::size_t idir=0; idir<3; ++idir)
    {
      // rhs vector
      for (ncomp_t c=0; c<ncomp; ++c)
      {
        auto mark = c*rdof;
        rhs_ls[el][c][idir] +=
          wdeltax[idir] * (W(er,mark,offset)-W(el,mark,offset));
        rhs_ls[er][c][idir] +=
          wdeltax[idir] * (W(er,mark,offset)-W(el,mark,offset));
      }
    }
  }
}

void
tk::bndLeastSq_P0P1( ncomp_t system,
                     ncomp_t ncomp,
                     ncomp_t offset,
                     std::size_t rdof,
                     const std::vector< bcconf_t >& bcconfig,
                     const inciter::FaceData& fd,
                     const Fields& geoFace,
                     const Fields& geoElem,
                     real t,
                     const StateFn& state,
                     const Fields& W,
                     std::vector< std::vector< std::array< real, 3 > > >& rhs_ls,
                     std::size_t nappend,
                     bool isConserved )
// *****************************************************************************
//  Compute boundary face contributions to the least-squares reconstruction
//! \param[in] system Equation system index
//! \param[in] ncomp Number of scalar components in this PDE system
//! \param[in] offset Offset this PDE system operates from
//! \param[in] rdof Maximum number of reconstructed degrees of freedom
//! \param[in] bcconfig BC configuration vector for multiple side sets
//! \param[in] fd Face connectivity and boundary conditions object
//! \param[in] geoFace Face geometry array
//! \param[in] geoElem Element geometry array
//! \param[in] t Physical time
//! \param[in] state Function to evaluate the left and right solution state at
//!   boundaries
//! \param[in] W Solution vector to be reconstructed at recent time step
//! \param[in,out] rhs_ls RHS reconstruction vector
//! \param[in] nappend If conserved variables are being reconstructed, this is
//!   the number of primitive quantities stored for this PDE system. If
//!   primitive quantities are being reconstructed, this is the number of
//!   conserved quantities stored for this system. This is necessary to extend
//!   the state vector to the right size, so that correct boundary conditions
//!   are obtained.
//!   A default is set to 0, so that calling code for systems that do not store
//!   primitive quantities does not need to specify this argument.
//! \param[in] isConserved Boolean which is true if conserved variables are
//!   being reconstructed. Default is true, so that it can be left unspecified
//!   by the calling code.
// *****************************************************************************
{
  const auto& bface = fd.Bface();
  const auto& esuf = fd.Esuf();

  for (const auto& s : bcconfig) {       // for all bc sidesets
    auto bc = bface.find( std::stoi(s) );// faces for side set
    if (bc != end(bface))
    {
      // Compute boundary face contributions
      for (const auto& f : bc->second)
      {
        Assert( esuf[2*f+1] == -1, "physical boundary element not -1" );

        std::size_t el = static_cast< std::size_t >(esuf[2*f]);

        // arrays for quadrature points
        std::array< real, 3 >
          fc{{ geoFace(f,4,0), geoFace(f,5,0), geoFace(f,6,0) }};
        std::array< real, 3 >
          fn{{ geoFace(f,1,0), geoFace(f,2,0), geoFace(f,3,0) }};

        // Compute the state variables at the left element
        std::vector< real >B(1,1.0);
        auto ul = eval_state( ncomp, offset, rdof, 1, el, W, B );
        std::vector< real >uappend(nappend,0.0);

        auto nsize = ncomp;

        if (isConserved) {
          // consolidate primitives into state vector
          ul.insert(ul.end(), uappend.begin(), uappend.end());
        }
        else {
          // consolidate conserved quantities into state vector
          ul.insert(ul.begin(), uappend.begin(), uappend.end());
          nsize = nappend;
        }

        Assert( ul.size() == ncomp+nappend, "Incorrect size for "
                "appended state vector" );

        // Compute the state at the face-center using BC
        auto ustate = state( system, nsize, ul, fc[0], fc[1], fc[2], t, fn );

        std::array< real, 3 > wdeltax{{ fc[0]-geoElem(el,1,0),
                                        fc[1]-geoElem(el,2,0),
                                        fc[2]-geoElem(el,3,0) }};

        for (std::size_t idir=0; idir<3; ++idir)
        {
          // rhs vector
          for (ncomp_t c=0; c<ncomp; ++c) 
          {
            auto cp = c;
            if (!isConserved) cp = ustate[0].size()-ncomp+c;
            rhs_ls[el][c][idir] +=
              wdeltax[idir] * (ustate[1][cp]-ustate[0][cp]);
          }
        }
      }
    }
  }
}

void
tk::solveLeastSq_P0P1( ncomp_t ncomp,
  ncomp_t offset,
  const std::size_t rdof,
  const std::vector< std::array< std::array< real, 3 >, 3 > >& lhs,
  const std::vector< std::vector< std::array< real, 3 > > >& rhs,
  Fields& W )
// *****************************************************************************
//  Solve 3x3 system for least-squares reconstruction
//! \param[in] ncomp Number of scalar components in this PDE system
//! \param[in] offset Offset this PDE system operates from
//! \param[in] rdof Maximum number of reconstructed degrees of freedom
//! \param[in] lhs LHS reconstruction matrix
//! \param[in] rhs RHS reconstruction vector
//! \param[in,out] W Solution vector to be reconstructed at recent time step
// *****************************************************************************
{
  for (std::size_t e=0; e<lhs.size(); ++e)
  {
    for (ncomp_t c=0; c<ncomp; ++c)
    {
      auto mark = c*rdof;

      // solve system using Cramer's rule
      auto ux = tk::cramer( lhs[e], rhs[e][c] );

      W(e,mark+1,offset) = ux[0];
      W(e,mark+2,offset) = ux[1];
      W(e,mark+3,offset) = ux[2];
    }
  }
}

void
tk::transform_P0P1( ncomp_t ncomp,
                    ncomp_t offset,
                    std::size_t rdof,
                    std::size_t nelem,
                    const std::vector< std::size_t >& inpoel,
                    const UnsMesh::Coords& coord,
                    Fields& W )
// *****************************************************************************
//  Transform the reconstructed P1-derivatives to the Dubiner dofs
//! \param[in] ncomp Number of scalar components in this PDE system
//! \param[in] offset Index for equation systems
//! \param[in] rdof Total number of reconstructed dofs
//! \param[in] nelem Total number of elements
//! \param[in] inpoel Element-node connectivity
//! \param[in] coord Array of nodal coordinates
//! \param[in,out] W Second-order reconstructed vector which gets transformed to
//!   the Dubiner reference space
// *****************************************************************************
{
  const auto& cx = coord[0];
  const auto& cy = coord[1];
  const auto& cz = coord[2];

  for (std::size_t e=0; e<nelem; ++e)
  {
    // Extract the element coordinates
    std::array< std::array< real, 3>, 4 > coordel {{
      {{ cx[ inpoel[4*e  ] ], cy[ inpoel[4*e  ] ], cz[ inpoel[4*e  ] ] }},
      {{ cx[ inpoel[4*e+1] ], cy[ inpoel[4*e+1] ], cz[ inpoel[4*e+1] ] }},
      {{ cx[ inpoel[4*e+2] ], cy[ inpoel[4*e+2] ], cz[ inpoel[4*e+2] ] }},
      {{ cx[ inpoel[4*e+3] ], cy[ inpoel[4*e+3] ], cz[ inpoel[4*e+3] ] }}
    }};

    auto jacInv =
      tk::inverseJacobian( coordel[0], coordel[1], coordel[2], coordel[3] );

    // Compute the derivatives of basis function for DG(P1)
    auto dBdx = tk::eval_dBdx_p1( rdof, jacInv );

    for (ncomp_t c=0; c<ncomp; ++c)
    {
      auto mark = c*rdof;

      // solve system using Cramer's rule
      auto ux = tk::cramer( {{ {{dBdx[0][1], dBdx[0][2], dBdx[0][3]}},
                               {{dBdx[1][1], dBdx[1][2], dBdx[1][3]}},
                               {{dBdx[2][1], dBdx[2][2], dBdx[2][3]}} }},
                            {{ W(e,mark+1,offset),
                               W(e,mark+2,offset),
                               W(e,mark+3,offset) }} );

      // replace physical derivatives with transformed dofs
      W(e,mark+1,offset) = ux[0];
      W(e,mark+2,offset) = ux[1];
      W(e,mark+3,offset) = ux[2];
    }
  }
}
