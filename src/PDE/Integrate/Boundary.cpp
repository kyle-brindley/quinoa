// *****************************************************************************
/*!
  \file      src/PDE/Integrate/Boundary.cpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019-2021 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Functions for computing physical boundary surface integrals of a
     system of PDEs in DG methods
  \details   This file contains functionality for computing physical boundary
     surface integrals of a system of PDEs used in discontinuous Galerkin
     methods for various orders of numerical representation.
*/
// *****************************************************************************

#include <array>

#include "Basis.hpp"
#include "Boundary.hpp"
#include "Vector.hpp"
#include "Quadrature.hpp"
#include "MultiMatTerms.hpp"
#include "MultiMat/MultiMatIndexing.hpp"
#include "Reconstruction.hpp"

namespace tk {

void
bndSurfInt( ncomp_t system,
            const bool pref,
            std::size_t nmat,
            const std::vector< inciter::EOS >& mat_blk,
            const std::size_t ndof,
            const std::size_t rdof,
            const std::vector< bcconf_t >& bcconfig,
            const inciter::FaceData& fd,
            const Fields& geoFace,
            const Fields& geoElem,
            const std::vector< std::size_t >& inpoel,
            const UnsMesh::Coords& coord,
            real t,
            const RiemannFluxFn& flux,
            const VelFn& vel,
            const StateFn& state,
            const Fields& U,
            const Fields& P,
            const std::vector< std::size_t >& ndofel,
            Fields& R,
            std::vector< std::vector< tk::real > >&,
            std::vector< std::vector< tk::real > >&,
            std::vector< std::vector< tk::real > >& riemannDeriv,
            int intsharp )
// *****************************************************************************
//! Compute boundary surface flux integrals for a given boundary type for DG
//! \details This function computes contributions from surface integrals along
//!   all faces for a particular boundary condition type, configured by the state
//!   function
//! \param[in] system Equation system index
//! \param[in] pref Indicator for p-adaptive algorithm
//! \param[in] nmat Number of materials in this PDE system
//! \param[in] mat_blk EOS material block
//! \param[in] ndof Maximum number of degrees of freedom
//! \param[in] rdof Maximum number of reconstructed degrees of freedom
//! \param[in] bcconfig BC configuration vector for multiple side sets
//! \param[in] fd Face connectivity and boundary conditions object
//! \param[in] geoFace Face geometry array
//! \param[in] geoElem Element geometry array
//! \param[in] inpoel Element-node connectivity
//! \param[in] coord Array of nodal coordinates
//! \param[in] t Physical time
//! \param[in] flux Riemann flux function to use
//! \param[in] vel Function to use to query prescribed velocity (if any)
//! \param[in] state Function to evaluate the left and right solution state at
//!   boundaries
//! \param[in] U Solution vector at recent time step
//! \param[in] P Vector of primitives at recent time step
//! \param[in] ndofel Vector of local number of degrees of freedom
//! \param[in,out] R Right-hand side vector computed
//! \param[in,out] vriem Vector of the riemann velocity
//! \param[in,out] riemannLoc Vector of coordinates where Riemann velocity data
//!   is available
//! \param[in,out] riemannDeriv Derivatives of partial-pressures and velocities
//!   computed from the Riemann solver for use in the non-conservative terms.
//!   These derivatives are used only for multi-material hydro and unused for
//!   single-material compflow and linear transport.
//! \param[in] intsharp Interface compression tag, an optional argument, with
//!   default 0, so that it is unused for single-material and transport.
// *****************************************************************************
{
  const auto& bface = fd.Bface();
  const auto& esuf = fd.Esuf();
  const auto& inpofa = fd.Inpofa();

  const auto& cx = coord[0];
  const auto& cy = coord[1];
  const auto& cz = coord[2];

  auto ncomp = U.nprop()/rdof;
  auto nprim = P.nprop()/rdof;

  //Assert( (nmat==1 ? riemannDeriv.empty() : true), "Non-empty Riemann "
  //        "derivative vector for single material compflow" );

  for (const auto& s : bcconfig) {       // for all bc sidesets
    auto bc = bface.find( std::stoi(s) );// faces for side set
    if (bc != end(bface))
    {
      for (const auto& f : bc->second)
      {
        Assert( esuf[2*f+1] == -1, "outside boundary element not -1" );

        std::size_t el = static_cast< std::size_t >(esuf[2*f]);

        auto ng = tk::NGfa(ndofel[el]);

        // arrays for quadrature points
        std::array< std::vector< real >, 2 > coordgp;
        std::vector< real > wgp;

        coordgp[0].resize( ng );
        coordgp[1].resize( ng );
        wgp.resize( ng );

        // get quadrature point weights and coordinates for triangle
        GaussQuadratureTri( ng, coordgp, wgp );

        // Extract the left element coordinates
        std::array< std::array< tk::real, 3>, 4 > coordel_l {{
        {{ cx[ inpoel[4*el  ] ], cy[ inpoel[4*el  ] ], cz[ inpoel[4*el  ] ] }},
        {{ cx[ inpoel[4*el+1] ], cy[ inpoel[4*el+1] ], cz[ inpoel[4*el+1] ] }},
        {{ cx[ inpoel[4*el+2] ], cy[ inpoel[4*el+2] ], cz[ inpoel[4*el+2] ] }},
        {{ cx[ inpoel[4*el+3] ], cy[ inpoel[4*el+3] ], cz[ inpoel[4*el+3] ] }} }};

        // Compute the determinant of Jacobian matrix
        auto detT_l =
          Jacobian( coordel_l[0], coordel_l[1], coordel_l[2], coordel_l[3] );

        // Extract the face coordinates
        std::array< std::array< tk::real, 3>, 3 > coordfa {{
          {{ cx[ inpofa[3*f  ] ], cy[ inpofa[3*f  ] ], cz[ inpofa[3*f  ] ] }},
          {{ cx[ inpofa[3*f+1] ], cy[ inpofa[3*f+1] ], cz[ inpofa[3*f+1] ] }},
          {{ cx[ inpofa[3*f+2] ], cy[ inpofa[3*f+2] ], cz[ inpofa[3*f+2] ] }} }};

        std::array< real, 3 >
          fn{{ geoFace(f,1), geoFace(f,2), geoFace(f,3) }};

        // Gaussian quadrature
        for (std::size_t igp=0; igp<ng; ++igp)
        {
          // Compute the coordinates of quadrature point at physical domain
          auto gp = eval_gp( igp, coordfa, coordgp );

          // If an rDG method is set up (P0P1), then, currently we compute the P1
          // basis functions and solutions by default. This implies that P0P1 is
          // unsupported in the p-adaptive DG (PDG). This is a workaround until
          // we have rdofel, which is needed to distinguish between ndofs and
          // rdofs per element for pDG.
          std::size_t dof_el;
          if (rdof > ndof)
          {
            dof_el = rdof;
          }
          else
          {
            dof_el = ndofel[el];
          }

          // For multi-material p-adaptive simulation, when dofel = 1, p0p1 is applied and ndof
          // for solution evaluation should be 4
          if(ncomp > 5 && dof_el == 1 && pref)
            dof_el = 4;

          std::array< tk::real, 3> ref_gp_l{
            Jacobian( coordel_l[0], gp, coordel_l[2], coordel_l[3] ) / detT_l,
            Jacobian( coordel_l[0], coordel_l[1], gp, coordel_l[3] ) / detT_l,
            Jacobian( coordel_l[0], coordel_l[1], coordel_l[2], gp ) / detT_l };

          //Compute the basis functions for the left element
          auto B_l = eval_basis( dof_el, ref_gp_l[0], ref_gp_l[1], ref_gp_l[2] );

          auto wt = wgp[igp] * geoFace(f,0);

          // Compute the state variables at the left element
          auto ugp = evalPolynomialSol(system, mat_blk, intsharp, ncomp, nprim,
            rdof, nmat, el, dof_el, inpoel, coord, geoElem, ref_gp_l, B_l, U, P);

          Assert( ugp.size() == ncomp+nprim, "Incorrect size for "
                  "appended boundary state vector" );

          auto var = state( system, ncomp, mat_blk, ugp, gp[0], gp[1], gp[2], t,
                            fn );

          // Compute the numerical flux
          auto fl = flux( mat_blk, fn, var, vel( system, ncomp, gp[0], gp[1],
                          gp[2], t ) );

          // Add the surface integration term to the rhs
          update_rhs_bc( ncomp, nmat, ndof, ndofel[el], wt, fn, el, fl,
                         B_l, R, riemannDeriv );
        }
      }
    }
  }
}

void
update_rhs_bc ( ncomp_t ncomp,
                std::size_t nmat,
                const std::size_t ndof,
                const std::size_t ndof_l,
                const tk::real wt,
                const std::array< tk::real, 3 >& fn,
                const std::size_t el,
                const std::vector< tk::real >& fl,
                const std::vector< tk::real >& B_l,
                Fields& R,
                std::vector< std::vector< tk::real > >& riemannDeriv )
// *****************************************************************************
//  Update the rhs by adding the boundary surface integration term
//! \param[in] ncomp Number of scalar components in this PDE system
//! \param[in] nmat Number of materials in this PDE system
//! \param[in] ndof Maximum number of degrees of freedom
//! \param[in] ndof_l Number of degrees of freedom for the left element
//! \param[in] wt Weight of gauss quadrature point
//! \param[in] fn Face/Surface normal
//! \param[in] el Left element index
//! \param[in] fl Surface flux
//! \param[in] B_l Basis function for the left element
//! \param[in,out] R Right-hand side vector computed
//! \param[in,out] riemannDeriv Derivatives of partial-pressures and velocities
//!   computed from the Riemann solver for use in the non-conservative terms.
//!   These derivatives are used only for multi-material hydro and unused for
//!   single-material compflow and linear transport.
// *****************************************************************************
{
  // following line commented until rdofel is made available.
  //Assert( B_l.size() == ndof_l, "Size mismatch" );

  for (ncomp_t c=0; c<ncomp; ++c)
  {
    auto mark = c*ndof;
    R(el, mark) -= wt * fl[c];

    if(ndof_l > 1)          //DG(P1)
    {
      R(el, mark+1) -= wt * fl[c] * B_l[1];
      R(el, mark+2) -= wt * fl[c] * B_l[2];
      R(el, mark+3) -= wt * fl[c] * B_l[3];
    }

    if(ndof_l > 4)          //DG(P2)
    {
      R(el, mark+4) -= wt * fl[c] * B_l[4];
      R(el, mark+5) -= wt * fl[c] * B_l[5];
      R(el, mark+6) -= wt * fl[c] * B_l[6];
      R(el, mark+7) -= wt * fl[c] * B_l[7];
      R(el, mark+8) -= wt * fl[c] * B_l[8];
      R(el, mark+9) -= wt * fl[c] * B_l[9];
    }
  }

  // Prep for non-conservative terms in multimat
  if (fl.size() > ncomp)
  {
    // Gradients of partial pressures
    for (std::size_t k=0; k<nmat; ++k)
    {
      for (std::size_t idir=0; idir<3; ++idir)
        riemannDeriv[3*k+idir][el] += wt * fl[ncomp+k] * fn[idir];
    }

    // Divergence of velocity multiples basis fucntion( d(uB) / dx )
    for(std::size_t idof = 0; idof < ndof_l; idof++)
      riemannDeriv[3*nmat+idof][el] += wt * fl[ncomp+nmat] * B_l[idof];
  }
}

void
bndSurfIntFV( ncomp_t system,
  std::size_t nmat,
  const std::vector< inciter::EOS >& mat_blk,
  const std::size_t rdof,
  const std::vector< bcconf_t >& bcconfig,
  const inciter::FaceData& fd,
  const Fields& geoFace,
  const Fields& geoElem,
  const std::vector< std::size_t >& inpoel,
  const UnsMesh::Coords& coord,
  real t,
  const RiemannFluxFn& flux,
  const VelFn& vel,
  const StateFn& state,
  const Fields& U,
  const Fields& P,
  Fields& R,
  std::vector< std::vector< tk::real > >& riemannDeriv,
  int intsharp )
// *****************************************************************************
//! Compute boundary surface flux integrals for a given boundary type for FV
//! \details This function computes contributions from surface integrals along
//!   all faces for a particular boundary condition type, configured by the state
//!   function
//! \param[in] system Equation system index
//! \param[in] nmat Number of materials in this PDE system
//! \param[in] mat_blk EOS material block
//! \param[in] rdof Maximum number of reconstructed degrees of freedom
//! \param[in] bcconfig BC configuration vector for multiple side sets
//! \param[in] fd Face connectivity and boundary conditions object
//! \param[in] geoFace Face geometry array
//! \param[in] geoElem Element geometry array
//! \param[in] inpoel Element-node connectivity
//! \param[in] coord Array of nodal coordinates
//! \param[in] t Physical time
//! \param[in] flux Riemann flux function to use
//! \param[in] vel Function to use to query prescribed velocity (if any)
//! \param[in] state Function to evaluate the left and right solution state at
//!   boundaries
//! \param[in] U Solution vector at recent time step
//! \param[in] P Vector of primitives at recent time step
//! \param[in,out] R Right-hand side vector computed
//! \param[in,out] riemannDeriv Derivatives of partial-pressures and velocities
//!   computed from the Riemann solver for use in the non-conservative terms.
//!   These derivatives are used only for multi-material hydro and unused for
//!   single-material compflow and linear transport.
//! \param[in] intsharp Interface compression tag, an optional argument, with
//!   default 0, so that it is unused for single-material and transport.
// *****************************************************************************
{
  const auto& bface = fd.Bface();
  const auto& esuf = fd.Esuf();

  const auto& cx = coord[0];
  const auto& cy = coord[1];
  const auto& cz = coord[2];

  auto ncomp = U.nprop()/rdof;
  auto nprim = P.nprop()/rdof;

  //Assert( (nmat==1 ? riemannDeriv.empty() : true), "Non-empty Riemann "
  //        "derivative vector for single material compflow" );

  for (const auto& s : bcconfig) {       // for all bc sidesets
    auto bc = bface.find( std::stoi(s) );// faces for side set
    if (bc != end(bface))
    {
      for (const auto& f : bc->second)
      {
        Assert( esuf[2*f+1] == -1, "outside boundary element not -1" );

        std::size_t el = static_cast< std::size_t >(esuf[2*f]);

        // Extract the left element coordinates
        std::array< std::array< tk::real, 3>, 4 > coordel_l {{
        {{ cx[ inpoel[4*el  ] ], cy[ inpoel[4*el  ] ], cz[ inpoel[4*el  ] ] }},
        {{ cx[ inpoel[4*el+1] ], cy[ inpoel[4*el+1] ], cz[ inpoel[4*el+1] ] }},
        {{ cx[ inpoel[4*el+2] ], cy[ inpoel[4*el+2] ], cz[ inpoel[4*el+2] ] }},
        {{ cx[ inpoel[4*el+3] ], cy[ inpoel[4*el+3] ], cz[ inpoel[4*el+3] ] }} }};

        // Compute the determinant of Jacobian matrix
        auto detT_l =
          Jacobian( coordel_l[0], coordel_l[1], coordel_l[2], coordel_l[3] );

        // face normal
        std::array< real, 3 >
          fn{{ geoFace(f,1), geoFace(f,2), geoFace(f,3) }};

        // face centroid
        std::array< real, 3 >
          gp{{ geoFace(f,4), geoFace(f,5), geoFace(f,6) }};

        std::array< tk::real, 3> ref_gp_l{
          Jacobian( coordel_l[0], gp, coordel_l[2], coordel_l[3] ) / detT_l,
          Jacobian( coordel_l[0], coordel_l[1], gp, coordel_l[3] ) / detT_l,
          Jacobian( coordel_l[0], coordel_l[1], coordel_l[2], gp ) / detT_l };

        //Compute the basis functions for the left element
        auto B_l = eval_basis( rdof, ref_gp_l[0], ref_gp_l[1], ref_gp_l[2] );

        // Compute the state variables at the left element
        auto ugp = evalPolynomialSol(system, mat_blk, intsharp, ncomp, nprim,
          rdof, nmat, el, rdof, inpoel, coord, geoElem, ref_gp_l, B_l, U, P);

        Assert( ugp.size() == ncomp+nprim, "Incorrect size for "
                "appended boundary state vector" );

        auto var = state( system, ncomp, mat_blk, ugp, gp[0], gp[1], gp[2], t,
                          fn );

        // Compute the numerical flux
        auto fl = flux( mat_blk, fn, var, vel( system, ncomp, gp[0], gp[1],
                        gp[2], t ) );

        // Add the surface integration term to the rhs
        for (ncomp_t c=0; c<ncomp; ++c)
        {
          R(el, c) -= geoFace(f,0) * fl[c];
        }
      
        // Prep for non-conservative terms in multimat
        if (fl.size() > ncomp)
        {
          // Gradients of partial pressures
          for (std::size_t k=0; k<nmat; ++k)
          {
            for (std::size_t idir=0; idir<3; ++idir)
              riemannDeriv[3*k+idir][el] += geoFace(f,0) * fl[ncomp+k]
              * fn[idir];
          }
      
          // Divergence of velocity
          riemannDeriv[3*nmat][el] += geoFace(f,0) * fl[ncomp+nmat];
        }
      }
    }
  }
}

} // tk::
