// *****************************************************************************
/*!
  \file      src/PDE/Riemann/LaxFriedrichsSolids.hpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019-2023 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Lax-Friedrichs Riemann flux function for solids
  \details   This file implements the Lax-Friedrichs Riemann solver for solids.
*/
// *****************************************************************************
#ifndef LaxFriedrichsSolids_h
#define LaxFriedrichsSolids_h

#include <vector>

#include "Types.hpp"
#include "Fields.hpp"
#include "Tags.hpp"
#include "FunctionPrototypes.hpp"
#include "Inciter/Options/Flux.hpp"
#include "EoS/EOS.hpp"
#include "MultiMat/MultiMatIndexing.hpp"
#include "MultiMat/MiscMultiMatFns.hpp"

namespace inciter {

//! Lax-Friedrichs approximate Riemann solver for solids
struct LaxFriedrichsSolids {

  //! Lax-Friedrichs approximate Riemann solver flux function
  //! \param[in] fn Face/Surface normal
  //! \param[in] u Left and right unknown/state vector
  //! \return Riemann flux solution according to Lax-Friedrichs, appended by
  //!   Riemann velocities and volume-fractions.
  //! \note The function signature must follow tk::RiemannFluxFn
  static tk::RiemannFluxFn::result_type
  flux( const std::vector< EOS >& mat_blk,
        const std::array< tk::real, 3 >& fn,
        const std::array< std::vector< tk::real >, 2 >& u,
        const std::vector< std::array< tk::real, 3 > >& = {} )
  {
    const auto nmat =
      g_inputdeck.get< tag::param, tag::multimat, tag::nmat >()[0];
    const auto& solidx = g_inputdeck.get< tag::param, tag::multimat,
      tag::matidxmap >().template get< tag::solidx >();

    auto ncomp = u[0].size()-(3+nmat);
    std::vector< tk::real > flx( ncomp, 0 ), fluxl(ncomp, 0), fluxr(ncomp,0);

    // Primitive variables
    tk::real rhol(0.0), rhor(0.0);
    for (std::size_t k=0; k<nmat; ++k)
    {
      rhol += u[0][densityIdx(nmat, k)];
      rhor += u[1][densityIdx(nmat, k)];
    }

    // Independently limited velocities for advection
    auto ul = u[0][ncomp+velocityIdx(nmat, 0)];
    auto vl = u[0][ncomp+velocityIdx(nmat, 1)];
    auto wl = u[0][ncomp+velocityIdx(nmat, 2)];
    auto ur = u[1][ncomp+velocityIdx(nmat, 0)];
    auto vr = u[1][ncomp+velocityIdx(nmat, 1)];
    auto wr = u[1][ncomp+velocityIdx(nmat, 2)];

    // define rotation matrix
    std::vector< std::array< tk::real, 3 > > rotMat;
    tk::real nx = fn[0];
    tk::real ny = fn[1];
    tk::real nz = fn[2];
    rotMat[0][0] = nx;
    rotMat[0][1] = ny;
    rotMat[0][2] = nz;
    rotMat[1][0] = ny;
    rotMat[1][1] = -nx;
    rotMat[1][2] = 0.0;
    rotMat[2][0] = nx*nz;
    rotMat[2][1] = ny*nz;
    rotMat[2][2] = -(nx*nx+ny*ny);

				     
    std::vector< std::array< tk::real, 3 > > agn_l, agn_r;
    
    std::vector< tk::real > al_l(nmat, 0.0), al_r(nmat, 0.0),
                            pml(nmat, 0.0), pmr(nmat, 0.0),
                            am_l(nmat, 0.0),
                            am_r(nmat, 0.0);
    std::vector< std::array< std::array< tk::real, 3 >, 3 > > ag_l, ag_r,
      asig_l, asig_r;
    std::vector< std::array< tk::real, 3 > > asign_l, asign_r;
    std::array< tk::real, 3 > sign_l {{0, 0, 0}}, sign_r {{0, 0, 0}};
    for (std::size_t k=0; k<nmat; ++k)
    {
      // Left state
      al_l[k] = u[0][volfracIdx(nmat, k)];
      pml[k] = u[0][ncomp+pressureIdx(nmat, k)];

      // inv deformation gradient and Cauchy stress tensors
      ag_l.push_back(getDeformGrad(nmat, k, u[0]));
      asig_l.push_back(mat_blk[k].computeTensor< EOS::CauchyStress >(
        u[0][densityIdx(nmat, k)], ul, vl, wl, u[0][energyIdx(nmat, k)],
        al_l[k], k, ag_l[k]));

      // normal stress (traction) vector
      asign_l.push_back(tk::matvec(asig_l[k], fn));
      for (std::size_t i=0; i<3; ++i)
        sign_l[i] += asign_l[k][i];

      // rotate deformation gradient tensor for speed of sound in normal dir
      // g*R
      cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
		  3, 3, 3, 1.0, ag_l[k], 3, rotMat, 3, 0.0, agn_l, 3);
      // Rt*g*R
      cblas_dgemm(CblasRowMajor, CblasTrans, CblasNoTrans,
		  3, 3, 3, 1.0, rotMat, 3, agn_l, 3, 0.0, agn_l, 3);
      am_l[k] = mat_blk[k].compute< EOS::soundspeed >(
        u[0][densityIdx(nmat, k)], pml[k], al_l[k], k, tk::dot(asign_l[k],fn),
        agn_l[k] );

      // Right state
      al_r[k] = u[1][volfracIdx(nmat, k)];
      pmr[k] = u[1][ncomp+pressureIdx(nmat, k)];

      // inv deformation gradient and Cauchy stress tensors
      ag_r.push_back(getDeformGrad(nmat, k, u[1]));
      asig_r.push_back(mat_blk[k].computeTensor< EOS::CauchyStress >(
        u[1][densityIdx(nmat, k)], ur, vr, wr, u[1][energyIdx(nmat, k)],
        al_r[k], k, ag_r[k]));
      // normal stress (traction) vector
      asign_r.push_back(tk::matvec(asig_r[k], fn));
      for (std::size_t i=0; i<3; ++i)
        sign_l[i] += asign_l[k][i];

      // rotate deformation gradient tensor for speed of sound in normal dir
      // g*R
      cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
		  3, 3, 3, 1.0, ag_r[k], 3, rotMat, 3, 0.0, agn_r, 3);
      // Rt*g*R
      cblas_dgemm(CblasRowMajor, CblasTrans, CblasNoTrans,
		  3, 3, 3, 1.0, rotMat, 3, agn_r, 3, 0.0, agn_r, 3);
      am_r[k] = mat_blk[k].compute< EOS::soundspeed >(
        u[1][densityIdx(nmat, k)], pmr[k], al_r[k], k, tk::dot(asign_r[k],fn),
        agn_r[k] );
    }

    // Mixture speed of sound
    tk::real ac_l(0.0), ac_r(0.0);
    for (std::size_t k=0; k<nmat; ++k)
    {
      ac_l += (u[0][densityIdx(nmat,k)]*am_l[k]*am_l[k]);
      ac_r += (u[1][densityIdx(nmat,k)]*am_r[k]*am_r[k]);
    }
    ac_l = std::sqrt( ac_l/rhol );
    ac_r = std::sqrt( ac_r/rhor );

    // Face-normal velocities from advective velocities
    auto vnl = ul*fn[0] + vl*fn[1] + wl*fn[2];
    auto vnr = ur*fn[0] + vr*fn[1] + wr*fn[2];

    // Maximum eignevalue and Riemann velocity
    auto lambda = std::max(std::abs(vnl), std::abs(vnr)) + std::max(ac_l, ac_r);
    auto vriem = 0.5 * (vnl + vnr);

    // Conservative fluxes
    for (std::size_t k=0; k<nmat; ++k)
    {
      // Left fluxes
      fluxl[volfracIdx(nmat, k)] = vnl*al_l[k];
      fluxl[densityIdx(nmat, k)] = vnl*u[0][densityIdx(nmat, k)];
      fluxl[energyIdx(nmat, k)] = vnl*u[0][energyIdx(nmat, k)];
      for (std::size_t i=0; i<3; ++i) {
        fluxl[energyIdx(nmat, k)] -= u[0][ncomp+velocityIdx(nmat,i)] *
          asign_l[k][i];
      }

      // fluxes for inv deformation gradient tensor
      if (solidx[k] > 0) {
        for (std::size_t i=0; i<3; ++i)
          for (std::size_t j=0; j<3; ++j)
            fluxl[deformIdx(nmat,solidx[k],i,j)] =
              (ul*ag_l[k][i][0] + vl*ag_l[k][i][1] + wl*ag_l[k][i][2]) * fn[j];
      }

      // Right fluxes
      fluxr[volfracIdx(nmat, k)] = vnr*al_r[k];
      fluxr[densityIdx(nmat, k)] = vnr*u[1][densityIdx(nmat, k)];
      fluxr[energyIdx(nmat, k)] = vnr*u[1][energyIdx(nmat, k)];
      for (std::size_t i=0; i<3; ++i) {
        fluxr[energyIdx(nmat, k)] -= u[1][ncomp+velocityIdx(nmat,i)] *
          asign_r[k][i];
      }

      // fluxes for inv deformation gradient tensor
      if (solidx[k] > 0) {
        for (std::size_t i=0; i<3; ++i)
          for (std::size_t j=0; j<3; ++j)
            fluxr[deformIdx(nmat,solidx[k],i,j)] =
              (ur*ag_r[k][i][0] + vr*ag_r[k][i][1] + wr*ag_r[k][i][2]) * fn[j];
      }
    }

    for (std::size_t idir=0; idir<3; ++idir)
    {
      fluxl[momentumIdx(nmat, idir)] = vnl*u[0][momentumIdx(nmat, idir)]
        - sign_l[idir];
      fluxr[momentumIdx(nmat, idir)] = vnr*u[1][momentumIdx(nmat, idir)]
        - sign_r[idir];
    }

    // Numerical flux function
    for (std::size_t c=0; c<ncomp; ++c)
      flx[c] = 0.5 * (fluxl[c] + fluxr[c] - lambda*(u[1][c] - u[0][c]));

    // Store Riemann-advected partial pressures
    for (std::size_t k=0; k<nmat; ++k)
      flx.push_back( 0.5*(pml[k] + pmr[k]) );

    // Store Riemann velocity
    flx.push_back( vriem );

    Assert( flx.size() == (u[0].size()+nmat+1), "Size of multi-material flux "
            "vector incorrect" );

    return flx;
  }

  ////! Flux type accessor
  ////! \return Flux type
  //static ctr::FluxType type() noexcept {
  //  return ctr::FluxType::LaxFriedrichs; }
};

} // inciter::

#endif // LaxFriedrichsSolids_h
