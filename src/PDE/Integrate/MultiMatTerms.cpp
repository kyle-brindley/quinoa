// *****************************************************************************
/*!
  \file      src/PDE/Integrate/MultiMatTerms.cpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019-2020 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Functions for computing volume integrals of multi-material terms
     using DG methods
  \details   This file contains functionality for computing volume integrals of
     non-conservative and pressure relaxation terms that appear in the
     multi-material hydrodynamic equations, using the discontinuous Galerkin
     method for various orders of numerical representation.
*/
// *****************************************************************************

#include "MultiMatTerms.hpp"
#include "Vector.hpp"
#include "Quadrature.hpp"
#include "EoS/EoS.hpp"
#include "MultiMat/MultiMatIndexing.hpp"

namespace tk {

void
nonConservativeInt( [[maybe_unused]] ncomp_t system,
                    std::size_t nmat,
                    ncomp_t offset,
                    const std::size_t ndof,
                    const std::size_t rdof,
                    const std::size_t nelem,
                    const std::vector< std::size_t >& inpoel,
                    const UnsMesh::Coords& coord,
                    const Fields& geoElem,
                    const Fields& U,
                    const Fields& P,
                    const std::vector< std::vector< tk::real > >&
                      riemannDeriv,
                    const std::vector< std::vector< tk::real > >&
                      vriempoly,
                    const std::vector< std::size_t >& ndofel,
                    Fields& R )
// *****************************************************************************
//  Compute volume integrals for multi-material DG
//! \details This is called for multi-material DG, computing volume integrals of
//!   terms in the volume fraction and energy equations, which do not exist in
//!   the single-material flow formulation (for `CompFlow` DG). For further
//!   details see Pelanti, M., & Shyue, K. M. (2019). A numerical model for
//!   multiphase liquid–vapor–gas flows with interfaces and cavitation.
//!   International Journal of Multiphase Flow, 113, 208-230.
//! \param[in] system Equation system index
//! \param[in] nmat Number of materials in this PDE system
//! \param[in] offset Offset this PDE system operates from
//! \param[in] ndof Maximum number of degrees of freedom
//! \param[in] rdof Maximum number of reconstructed degrees of freedom
//! \param[in] nelem Total number of elements
//! \param[in] inpoel Element-node connectivity
//! \param[in] coord Array of nodal coordinates
//! \param[in] geoElem Element geometry array
//! \param[in] U Solution vector at recent time step
//! \param[in] P Vector of primitive quantities at recent time step
//! \param[in] riemannDeriv Derivatives of partial-pressures and velocities
//!   computed from the Riemann solver for use in the non-conservative terms
//! \param[in] vriempoly Vector of velocity polynomial solution
//! \param[in] ndofel Vector of local number of degrees of freedome
//! \param[in,out] R Right-hand side vector added to
// *****************************************************************************
{
  using inciter::volfracIdx;
  using inciter::densityIdx;
  using inciter::momentumIdx;
  using inciter::energyIdx;
  using inciter::velocityIdx;

  const auto& cx = coord[0];
  const auto& cy = coord[1];
  const auto& cz = coord[2];

  auto ncomp = U.nprop()/rdof;
  auto nprim = P.nprop()/rdof;

  // compute volume integrals
  for (std::size_t e=0; e<nelem; ++e)
  {
    auto ng = tk::NGvol(ndofel[e]);

    // arrays for quadrature points
    std::array< std::vector< real >, 3 > coordgp;
    std::vector< real > wgp;

    coordgp[0].resize( ng );
    coordgp[1].resize( ng );
    coordgp[2].resize( ng );
    wgp.resize( ng );

    GaussQuadratureTet( ng, coordgp, wgp );

    // Extract the element coordinates
    std::array< std::array< real, 3>, 4 > coordel {{
      {{ cx[ inpoel[4*e  ] ], cy[ inpoel[4*e  ] ], cz[ inpoel[4*e  ] ] }},
      {{ cx[ inpoel[4*e+1] ], cy[ inpoel[4*e+1] ], cz[ inpoel[4*e+1] ] }},
      {{ cx[ inpoel[4*e+2] ], cy[ inpoel[4*e+2] ], cz[ inpoel[4*e+2] ] }},
      {{ cx[ inpoel[4*e+3] ], cy[ inpoel[4*e+3] ], cz[ inpoel[4*e+3] ] }}
    }};

    auto jacInv =
            inverseJacobian( coordel[0], coordel[1], coordel[2], coordel[3] );

    // Compute the derivatives of basis function for DG(P1)
    std::array< std::vector<tk::real>, 3 > dBdx;
    if (ndofel[e] > 1)
      dBdx = eval_dBdx_p1( ndofel[e], jacInv );

    // Gaussian quadrature
    for (std::size_t igp=0; igp<ng; ++igp)
    {
      if (ndofel[e] > 4)
        eval_dBdx_p2( igp, coordgp, jacInv, dBdx );

      // If an rDG method is set up (P0P1), then, currently we compute the P1
      // basis functions and solutions by default. This implies that P0P1 is
      // unsupported in the p-adaptive DG (PDG).
      std::size_t dof_el;
      if (rdof > ndof)
      {
        dof_el = rdof;
      }
      else
      {
        dof_el = ndofel[e];
      }

      // Compute the basis function
      auto B =
        eval_basis( dof_el, coordgp[0][igp], coordgp[1][igp], coordgp[2][igp] );

      auto wt = wgp[igp] * geoElem(e, 0, 0);

      auto ugp = eval_state( ncomp, offset, rdof, dof_el, e, U, B );
      auto pgp = eval_state( nprim, offset, rdof, dof_el, e, P, B );

      // get bulk properties
      tk::real rhob(0.0);
      for (std::size_t k=0; k<nmat; ++k)
          rhob += ugp[densityIdx(nmat, k)];

      std::array< tk::real, 3 > vel{{ pgp[velocityIdx(nmat, 0)],
                                      pgp[velocityIdx(nmat, 1)],
                                      pgp[velocityIdx(nmat, 2)] }};

      std::vector< tk::real > ymat(nmat, 0.0);
      std::array< tk::real, 3 > dap{{0.0, 0.0, 0.0}};
      for (std::size_t k=0; k<nmat; ++k)
      {
        ymat[k] = ugp[densityIdx(nmat, k)]/rhob;

        for (std::size_t idir=0; idir<3; ++idir)
          dap[idir] += riemannDeriv[3*k+idir][e];
      }

      // compute non-conservative terms
      std::vector< tk::real > ncf(ncomp, 0.0);

      for (std::size_t idir=0; idir<3; ++idir)
        ncf[momentumIdx(nmat, idir)] = 0.0;

      for (std::size_t k=0; k<nmat; ++k)
      {
        ncf[densityIdx(nmat, k)] = 0.0;
        ncf[volfracIdx(nmat, k)] = ugp[volfracIdx(nmat, k)]
                                   * riemannDeriv[3*nmat][e];
        for (std::size_t idir=0; idir<3; ++idir)
          ncf[energyIdx(nmat, k)] -= vel[idir] * ( ymat[k]*dap[idir]
                                                - riemannDeriv[3*k+idir][e] );
      }

      // Evaluate the velocity used for the multi-material term integration for
      // volume fraction equation
      std::vector< tk::real> vriem(3, 0.0);
      if(ndofel[e] > 1)
      {
        auto gp = eval_gp( igp, coordel, coordgp );
        for(std::size_t idir = 0; idir < 3; idir++)
        {
          auto mark = idir * 4;
          vriem[idir] = vriempoly[e][mark];
          for(std::size_t k = 1; k < 4; k++)
            vriem[idir] += vriempoly[e][mark+k] * gp[k-1];
        }
      }

      update_rhs_ncn( ncomp, offset, nmat, ndof, ndofel[e], wt, e, ugp, B, dBdx,
                      riemannDeriv, vriem, ncf, R );
    }
  }
}

void
update_rhs_ncn(
  ncomp_t ncomp,
  ncomp_t offset,
  const std::size_t nmat,
  const std::size_t ndof,
  const std::size_t ndof_el,
  const tk::real wt,
  const std::size_t e,
  const std::vector<tk::real>& ugp,
  const std::vector<tk::real>& B,
  const std::array< std::vector<tk::real>, 3 >& dBdx,
  const std::vector< std::vector<tk::real> >& riemannDeriv,
  const std::vector< tk::real >& vriem,
  std::vector< tk::real >& ncf,
  Fields& R )
// *****************************************************************************
//  Update the rhs by adding the non-conservative term integrals
//! \param[in] ncomp Number of scalar components in this PDE system
//! \param[in] offset Offset this PDE system operates from
//! \param[in] nmat Number of materials
//! \param[in] ndof Maximum number of degrees of freedom
//! \param[in] ndof_el Number of degrees of freedom for local element
//! \param[in] wt Weight of gauss quadrature point
//! \param[in] e Element index
//! \param[in] ugp Conservative variables at local quadrature point
//! \param[in] B Basis function evaluated at local quadrature point
//! \param[in] dBdx Vector of basis function derivatives
//! \param[in] riemannDeriv Derivatives of partial-pressures and velocities
//! \param[in] ncf Vector of non-conservative terms
//! \param[in] vriem Velocity used for volume fraction equation
//! \param[in,out] R Right-hand side vector computed
// *****************************************************************************
{
  //Assert( dBdx[0].size() == ndof_el,
  //        "Size mismatch for basis function derivatives" );
  //Assert( dBdx[1].size() == ndof_el,
  //        "Size mismatch for basis function derivatives" );
  //Assert( dBdx[2].size() == ndof_el,
  //        "Size mismatch for basis function derivatives" );
  //Assert( ncf.size() == ncomp,
  //        "Size mismatch for non-conservative term" );
  Assert( ncf.size() == ncomp, "Size mismatch for non-conservative term" );

  for (ncomp_t c=0; c<ncomp; ++c)
  {
    auto mark = c*ndof;
    R(e, mark, offset) += wt * ncf[c];
  }

  if( ndof_el > 1)
  {
    std::vector< tk::real > ncf_volp1(3*nmat, 0.0);
    for (std::size_t k=0; k<nmat; ++k)
    {
      ncf_volp1[3*k  ] = ugp[inciter::volfracIdx(nmat, k)] * (riemannDeriv[3*nmat][e] * B[1]
        + vriem[0] * dBdx[0][1] + vriem[1] * dBdx[1][1] + vriem[2] * dBdx[2][1]);
      ncf_volp1[3*k+1] = ugp[inciter::volfracIdx(nmat, k)] * (riemannDeriv[3*nmat][e] * B[2]
        + vriem[0] * dBdx[0][2] + vriem[1] * dBdx[1][2] + vriem[2] * dBdx[2][2]);
      ncf_volp1[3*k+2] = ugp[inciter::volfracIdx(nmat, k)] * (riemannDeriv[3*nmat][e] * B[3]
        + vriem[0] * dBdx[0][3] + vriem[1] * dBdx[1][3] + vriem[2] * dBdx[2][3]);

      auto mark = k*ndof;
      R(e, mark+1, offset) += wt * ncf_volp1[3*k];
      R(e, mark+2, offset) += wt * ncf_volp1[3*k+1];
      R(e, mark+3, offset) += wt * ncf_volp1[3*k+2];
    }

    for (ncomp_t c=nmat; c<ncomp; ++c)
    {
      auto mark = c*ndof;
      R(e, mark+1, offset) += wt * ncf[c] * B[1];
      R(e, mark+2, offset) += wt * ncf[c] * B[2];
      R(e, mark+3, offset) += wt * ncf[c] * B[3];
    }
  }
}

void
pressureRelaxationInt( ncomp_t system,
                       std::size_t nmat,
                       ncomp_t offset,
                       const std::size_t ndof,
                       const std::size_t rdof,
                       const std::size_t nelem,
                       const Fields& geoElem,
                       const Fields& U,
                       const Fields& P,
                       const std::vector< std::size_t >& ndofel,
                       const tk::real ct,
                       Fields& R )
// *****************************************************************************
//  Compute volume integrals of pressure relaxation terms in multi-material DG
//! \details This is called for multi-material DG to compute volume integrals of
//!   finite pressure relaxation terms in the volume fraction and energy
//!   equations, which do not exist in the single-material flow formulation (for
//!   `CompFlow` DG). For further details see Dobrev, V. A., Kolev, T. V.,
//!   Rieben, R. N., & Tomov, V. Z. (2016). Multi‐material closure model for
//!   high‐order finite element Lagrangian hydrodynamics. International Journal
//!   for Numerical Methods in Fluids, 82(10), 689-706.
//! \param[in] system Equation system index
//! \param[in] nmat Number of materials in this PDE system
//! \param[in] offset Offset this PDE system operates from
//! \param[in] ndof Maximum number of degrees of freedom
//! \param[in] rdof Maximum number of reconstructed degrees of freedom
//! \param[in] nelem Total number of elements
//! \param[in] geoElem Element geometry array
//! \param[in] U Solution vector at recent time step
//! \param[in] P Vector of primitive quantities at recent time step
//! \param[in] ndofel Vector of local number of degrees of freedome
//! \param[in] ct Pressure relaxation time-scale for this system
//! \param[in,out] R Right-hand side vector added to
// *****************************************************************************
{
  using inciter::volfracIdx;
  using inciter::densityIdx;
  using inciter::momentumIdx;
  using inciter::energyIdx;
  using inciter::pressureIdx;
  using inciter::velocityIdx;

  auto ncomp = U.nprop()/rdof;
  auto nprim = P.nprop()/rdof;

  // compute volume integrals
  for (std::size_t e=0; e<nelem; ++e)
  {
    auto dx = geoElem(e,4,0)/2.0;
    auto ng = NGvol(ndofel[e]);

    // arrays for quadrature points
    std::array< std::vector< real >, 3 > coordgp;
    std::vector< real > wgp;

    coordgp[0].resize( ng );
    coordgp[1].resize( ng );
    coordgp[2].resize( ng );
    wgp.resize( ng );

    GaussQuadratureTet( ng, coordgp, wgp );

    // Compute the derivatives of basis function for DG(P1)
    std::array< std::vector<real>, 3 > dBdx;

    // Gaussian quadrature
    for (std::size_t igp=0; igp<ng; ++igp)
    {
      // If an rDG method is set up (P0P1), then, currently we compute the P1
      // basis functions and solutions by default. This implies that P0P1 is
      // unsupported in the p-adaptive DG (PDG).
      std::size_t dof_el;
      if (rdof > ndof)
      {
        dof_el = rdof;
      }
      else
      {
        dof_el = ndofel[e];
      }

      // Compute the basis function
      auto B =
        eval_basis( dof_el, coordgp[0][igp], coordgp[1][igp], coordgp[2][igp] );

      auto wt = wgp[igp] * geoElem(e, 0, 0);

      auto ugp = eval_state( ncomp, offset, rdof, dof_el, e, U, B );
      auto pgp = eval_state( nprim, offset, rdof, dof_el, e, P, B );

      // get bulk properties
      real rhob(0.0);
      for (std::size_t k=0; k<nmat; ++k)
        rhob += ugp[densityIdx(nmat, k)];

      // get pressures and bulk modulii
      real pb(0.0), nume(0.0), deno(0.0), trelax(0.0);
      std::vector< real > apmat(nmat, 0.0), kmat(nmat, 0.0);
      for (std::size_t k=0; k<nmat; ++k)
      {
        real arhomat = ugp[densityIdx(nmat, k)];
        real alphamat = ugp[volfracIdx(nmat, k)];
        apmat[k] = pgp[pressureIdx(nmat, k)];
        real amat = inciter::eos_soundspeed< tag::multimat >( system, arhomat,
          apmat[k], alphamat, k );
        kmat[k] = arhomat * amat * amat;
        pb += apmat[k];

        // relaxation parameters
        trelax = std::max(trelax, ct*dx/amat);
        nume += alphamat * apmat[k] / kmat[k];
        deno += alphamat * alphamat / kmat[k];
      }
      auto p_relax = nume/deno;

      // compute pressure relaxation terms
      std::vector< real > s_prelax(ncomp, 0.0);
      for (std::size_t k=0; k<nmat; ++k)
      {
        auto s_alpha = (apmat[k]-p_relax*ugp[volfracIdx(nmat, k)])
          * (ugp[volfracIdx(nmat, k)]/kmat[k]) / trelax;
        s_prelax[volfracIdx(nmat, k)] = s_alpha;
        s_prelax[energyIdx(nmat, k)] = - pb*s_alpha;
      }

      update_rhs_pre( ncomp, offset, ndof, ndofel[e], wt, e, B, s_prelax, R );
    }
  }
}

void
update_rhs_pre(
  ncomp_t ncomp,
  ncomp_t offset,
  const std::size_t ndof,
  const std::size_t ndof_el,
  const tk::real wt,
  const std::size_t e,
  const std::vector< tk::real >& B,
  std::vector< tk::real >& ncf,
  Fields& R )
// *****************************************************************************
//  Update the rhs by adding the pressure relaxation integrals
//! \param[in] ncomp Number of scalar components in this PDE system
//! \param[in] offset Offset this PDE system operates from
//! \param[in] ndof Maximum number of degrees of freedom
//! \param[in] ndof_el Number of degrees of freedom for local element
//! \param[in] wt Weight of gauss quadrature point
//! \param[in] e Element index
//! \param[in] B Basis function evaluated at local quadrature point
//! \param[in] ncf Vector of non-conservative terms
//! \param[in,out] R Right-hand side vector computed
// *****************************************************************************
{
  //Assert( dBdx[0].size() == ndof_el,
  //        "Size mismatch for basis function derivatives" );
  //Assert( dBdx[1].size() == ndof_el,
  //        "Size mismatch for basis function derivatives" );
  //Assert( dBdx[2].size() == ndof_el,
  //        "Size mismatch for basis function derivatives" );
  //Assert( ncf.size() == ncomp,
  //        "Size mismatch for non-conservative term" );
  Assert( ncf.size() == ncomp, "Size mismatch for non-conservative term" );

  for (ncomp_t c=0; c<ncomp; ++c)
  {
    auto mark = c*ndof;
    R(e, mark, offset) += wt * ncf[c];
    if( ndof_el > 1)
    {
      R(e, mark+1, offset) += wt * ncf[c] * B[1];
      R(e, mark+2, offset) += wt * ncf[c] * B[2];
      R(e, mark+3, offset) += wt * ncf[c] * B[3];
    }
  }
}

void solvevriem( const std::size_t nelem,
                 const std::vector< std::vector< tk::real > >& vriem,
                 const std::vector< std::vector< tk::real > >& xcoord,
                 std::vector< std::vector< tk::real > >& vriempoly )
// *****************************************************************************
//  Solve the reconstruct velocity used for volume fraction equation by
//  Least square method
//! \param[in] nelem Numer of elements
//! \param[in,out] vriem Vector of the riemann velocity
//! \param[in,out] xcoord Vector of the coordinates of riemann velocity data
//! \param[in,out] vriempoly Vector of velocity polynomial solution
// *****************************************************************************
{
  for (std::size_t e=0; e<nelem; ++e)
  {
    // Use the normal method to construct the linear system A^T * A * x = u
    auto npoin = xcoord[e].size()/3;
    std::vector< std::vector< tk::real > > A(npoin, std::vector<tk::real>(4, 1.0));

    for(std::size_t k = 0; k < npoin; k++)
    {
      auto mark = k * 3;
      A[k][1] = xcoord[e][mark];
      A[k][2] = xcoord[e][mark+1];
      A[k][3] = xcoord[e][mark+2];
    }

    std::vector< std::vector< tk::real > > B(4, std::vector<tk::real>(4, 1.0));
    for(std::size_t i = 0; i < 4; i++)
      for(std::size_t j = 0; j < 4; j++)
        for(std::size_t k = 0; k < npoin; k++)
          B[i][j] += A[k][i] * A[k][j];

    for(std::size_t idir = 0; idir < 3; idir++)
    {
      std::vector<tk::real> x(4, 0.0);
      std::vector<tk::real> u(4, 0.0);

      std::vector<tk::real> vel(npoin, 1.0);
      for(std::size_t k = 0; k < npoin; k++)
      {
        auto mark = k * 3 + idir;
        vel[k] = vriem[e][mark];
      }
      for(std::size_t k = 0; k < 4; k++)
        for(std::size_t i = 0; i < npoin; i++)
          u[k] += A[i][k] * vel[i];
 
      // Solve the 4x4 linear system by LU method
      LU(4, B, u, x);

      auto idirmark = idir * 4;
      for(std::size_t k = 0; k < 4; k++)
        vriempoly[e][idirmark+k] = x[k];
    }
  }
}

}// tk::
