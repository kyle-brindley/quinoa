// *****************************************************************************
/*!
  \file      src/PDE/Limiter.hpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019-2021 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Limiters for discontiunous Galerkin methods
  \details   This file contains functions that provide limiter function
    calculations for maintaining monotonicity near solution discontinuities
    for the DG discretization.
*/
// *****************************************************************************
#ifndef Limiter_h
#define Limiter_h

#include "Types.hpp"
#include "Fields.hpp"
#include "UnsMesh.hpp"
#include "MultiMat/MultiMatIndexing.hpp"

namespace inciter {

using ncomp_t = kw::ncomp::info::expect::type;

//! Weighted Essentially Non-Oscillatory (WENO) limiter for DGP1
void
WENO_P1( const std::vector< int >& esuel,
         inciter::ncomp_t offset,
         tk::Fields& U );

//! Superbee limiter for DGP1
void
Superbee_P1( const std::vector< int >& esuel,
             const std::vector< std::size_t >& inpoel,
             const std::vector< std::size_t >& ndofel,
             inciter::ncomp_t offset,
             const tk::UnsMesh::Coords& coord,
             tk::Fields& U );

//! Superbee limiter for multi-material DGP1
void
SuperbeeMultiMat_P1(
  const std::vector< int >& esuel,
  const std::vector< std::size_t >& inpoel,
  const std::vector< std::size_t >& ndofel,
  std::size_t system,
  inciter::ncomp_t offset,
  const tk::UnsMesh::Coords& coord,
  tk::Fields& U,
  tk::Fields& P,
  std::size_t nmat );

//! Kuzmin's vertex-based limiter for transport DGP1
void
VertexBasedTransport_P1(
  const std::map< std::size_t, std::vector< std::size_t > >& esup,
  const std::vector< std::size_t >& inpoel,
  const std::vector< std::size_t >& ndofel,
  std::size_t nelem,
  std::size_t system,
  std::size_t offset,
  const tk::Fields& geoElem,
  const tk::UnsMesh::Coords& coord,
  tk::Fields& U );

//! Kuzmin's vertex-based limiter for single-material DGP1
void
VertexBasedCompflow_P1(
  const std::map< std::size_t, std::vector< std::size_t > >& esup,
  const std::vector< std::size_t >& inpoel,
  const std::vector< std::size_t >& ndofel,
  std::size_t nelem,
  std::size_t offset,
  const tk::Fields& geoElem,
  const tk::UnsMesh::Coords& coord,
  tk::Fields& U );

//! Kuzmin's vertex-based limiter for single-material DGP2
void
VertexBasedCompflow_P2(
  const std::map< std::size_t, std::vector< std::size_t > >& esup,
  const std::vector< std::size_t >& inpoel,
  const std::vector< std::size_t >& ndofel,
  std::size_t nelem,
  std::size_t offset,
  const tk::Fields& geoElem,
  const tk::UnsMesh::Coords& coord,
  const std::vector< std::size_t >& gid,
  const std::unordered_map< std::size_t, std::size_t >& bid,
  const std::vector< std::vector<tk::real> >& uNodalExtrm,
  tk::Fields& U );

//! Kuzmin's vertex-based limiter for multi-material DGP1
void
VertexBasedMultiMat_P1(
  const std::map< std::size_t, std::vector< std::size_t > >& esup,
  const std::vector< std::size_t >& inpoel,
  const std::vector< std::size_t >& ndofel,
  std::size_t nelem,
  std::size_t system,
  std::size_t offset,
  const tk::Fields& geoElem,
  const tk::UnsMesh::Coords& coord,
  tk::Fields& U,
  tk::Fields& P,
  std::size_t nmat );

//! Kuzmin's vertex-based limiter for multi-material FV
void
VertexBasedMultiMat_FV(
  const std::map< std::size_t, std::vector< std::size_t > >& esup,
  const std::vector< std::size_t >& inpoel,
  std::size_t nelem,
  std::size_t system,
  std::size_t offset,
  const tk::Fields& geoElem,
  const tk::UnsMesh::Coords& coord,
  tk::Fields& U,
  tk::Fields& P,
  std::size_t nmat );

//! WENO limiter function calculation for P1 dofs
void
WENOLimiting( const tk::Fields& U,
              const std::vector< int >& esuel,
              std::size_t e,
              inciter::ncomp_t c,
              std::size_t rdof,
              inciter::ncomp_t offset,
              tk::real cweight,
              std::array< std::vector< tk::real >, 3 >& limU );

//! Superbee limiter function calculation for P1 dofs
std::vector< tk::real >
SuperbeeLimiting( const tk::Fields& U,
                  const std::vector< int >& esuel,
                  const std::vector< std::size_t >& inpoel,
                  const tk::UnsMesh::Coords& coord,
                  std::size_t e,
                  std::size_t ndof,
                  std::size_t rdof,
                  std::size_t dof_el,
                  inciter::ncomp_t offset,
                  inciter:: ncomp_t ncomp,
                  tk::real beta_lim );

//! Kuzmin's vertex-based limiter function calculation for P1 dofs
std::vector< tk::real >
VertexBasedLimiting( const std::vector< std::vector< tk::real > >& unk,
  const tk::Fields& U,
  const std::map< std::size_t, std::vector< std::size_t > >& esup,
  const std::vector< std::size_t >& inpoel,
  const tk::UnsMesh::Coords& coord,
  const tk::Fields& geoElem,
  std::size_t e,
  std::size_t rdof,
  std::size_t ,
  std::size_t offset,
  std::size_t ncomp );

//! Kuzmin's vertex-based limiter function calculation for P2 dofs
std::vector< tk::real >
VertexBasedLimiting_P2( const std::vector< std::vector< tk::real > >& unk,
  const tk::Fields& U,
  const std::map< std::size_t, std::vector< std::size_t > >& esup,
  const std::vector< std::size_t >& inpoel,
  const tk::UnsMesh::Coords& coord,
  const tk::Fields& geoElem,
  std::size_t e,
  std::size_t rdof,
  std::size_t dof_el,
  std::size_t offset,
  std::size_t ncomp,
  const std::vector< std::size_t >& gid,
  const std::unordered_map< std::size_t, std::size_t >& bid,
  const std::vector< std::vector<tk::real> >& NodalExtrm );

//! Consistent limiter modifications for P1 dofs
void consistentMultiMatLimiting_P1( std::size_t nmat,
                                    ncomp_t offset,
                                    std::size_t rdof,
                                    std::size_t e,
                                    tk::Fields& U,
                                    tk::Fields& P,
                                    std::vector< tk::real >& phic,
                                    std::vector< tk::real >& phip );

//! Bound preserving limiter for the P1 dofs of volume fractions
void BoundPreservingLimiting( std::size_t nmat,
                              ncomp_t offset,
                              std::size_t ndof,
                              std::size_t e,
                              const std::vector< std::size_t >& inpoel,
                              const tk::UnsMesh::Coords& coord,
                              const tk::Fields& U,
                              std::vector< tk::real >& phic );

//! Interface indicator function, which checks element for material interface
bool
interfaceIndicator( std::size_t nmat,
  const std::vector< tk::real >& al,
  std::vector< std::size_t >& matInt );

//! Clean up the state of trace materials for multi-material PDE system
bool
cleanTraceMultiMat(
  std::size_t nelem,
  std::size_t system,
  std::size_t offset,
  const tk::Fields& geoElem,
  std::size_t nmat,
  tk::Fields& U,
  tk::Fields& P );

//! Time step restriction for multi material cell-centered schemes
tk::real
timeStepSizeMultiMat(
  const std::vector< int >& esuf,
  const tk::Fields& geoFace,
  const tk::Fields& geoElem,
  const std::size_t nelem,
  std::size_t offset,
  std::size_t nmat,
  const tk::Fields& U,
  const tk::Fields& P );

} // inciter::

#endif // Limiter_h
