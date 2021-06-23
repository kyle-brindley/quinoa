// *****************************************************************************
/*!
  \file      src/PDE/Integrate/Basis.cpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019-2021 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Functions for computing the Dubiner basis functions in DG methods
  \details   This file contains functionality for computing the basis functions
     and relating coordinates transformation functions used in discontinuous
     Galerkin methods for variaous orders of numerical representation. The basis
     functions chosen for the DG method are the Dubiner basis, which are
     Legendre polynomials modified for tetrahedra, which are defined only on
     the reference/master tetrahedron.
  \see [1] https://doi.org/10.1007/BF01060030
  \see [2] https://doi.org/10.1093/imamat/hxh111
*/
// *****************************************************************************

#include <array>

#include "Basis.hpp"

std::array< tk::real, 3 >
tk::eval_gp ( const std::size_t igp,
              const std::array< std::array< tk::real, 3>, 3 >& coordfa,
              const std::array< std::vector< tk::real >, 2 >& coordgp )
// *****************************************************************************
//  Compute the coordinates of quadrature points for face integral in physical
//  space
//! \param[in] igp Index of quadrature points
//! \param[in] coordfa Array of nodal coordinates for face element
//! \param[in] coordgp Array of coordinates for quadrature points in reference
//!   space
//! \return Array of coordinates for quadrature points in physical space
// *****************************************************************************
{
  // Barycentric coordinates for the triangular face
  auto shp1 = 1.0 - coordgp[0][igp] - coordgp[1][igp];
  auto shp2 = coordgp[0][igp];
  auto shp3 = coordgp[1][igp];

  // Transformation of the quadrature point from the 2D reference/master
  // element to physical space, to obtain its physical (x,y,z) coordinates.
  return {{ coordfa[0][0]*shp1 + coordfa[1][0]*shp2 + coordfa[2][0]*shp3,
            coordfa[0][1]*shp1 + coordfa[1][1]*shp2 + coordfa[2][1]*shp3,
            coordfa[0][2]*shp1 + coordfa[1][2]*shp2 + coordfa[2][2]*shp3 }};
}

std::array< tk::real, 3 >
tk::eval_gp ( const std::size_t igp,
              const std::array< std::array< tk::real, 3>, 4 >& coord,
              const std::array< std::vector< tk::real >, 3 >& coordgp )
// *****************************************************************************
//  Compute the coordinates of quadrature points for volume integral in
//  physical space
//! \param[in] igp Index of quadrature points
//! \param[in] coord Array of nodal coordinates for tetrahedron element
//! \param[in] coordgp Array of coordinates for quadrature points in reference space
//! \return Array of coordinates for quadrature points in physical space
// *****************************************************************************
{
  // Barycentric coordinates for the tetradedron element
  auto shp1 = 1.0 - coordgp[0][igp] - coordgp[1][igp] - coordgp[2][igp];
  auto shp2 = coordgp[0][igp];
  auto shp3 = coordgp[1][igp];
  auto shp4 = coordgp[2][igp];

  // Transformation of the quadrature point from the reference/master
  // element to physical space, to obtain its physical (x,y,z) coordinates.
  return {{
   coord[0][0]*shp1 + coord[1][0]*shp2 + coord[2][0]*shp3 + coord[3][0]*shp4,
   coord[0][1]*shp1 + coord[1][1]*shp2 + coord[2][1]*shp3 + coord[3][1]*shp4,
   coord[0][2]*shp1 + coord[1][2]*shp2 + coord[2][2]*shp3 + coord[3][2]*shp4 }};
}

std::array< std::vector<tk::real>, 3 >
tk::eval_dBdx_p1( const std::size_t ndof,
                  const std::array< std::array< tk::real, 3 >, 3 >& jacInv )
// *****************************************************************************
//  Compute the derivatives of basis functions for DG(P1)
//! \param[in] ndof Number of degrees of freedom
//! \param[in] jacInv Array of the inverse of Jacobian
//! \return Array of the derivatives of basis functions
// *****************************************************************************
{
  // The derivatives of the basis functions dB/dx are easily calculated
  // via a transformation to the reference space as,
  // dB/dx = dB/dxi . dxi/dx,
  // where, x = (x,y,z) are the physical coordinates, and
  //        xi = (xi, eta, zeta) are the reference coordinates.
  // The matrix dxi/dx is the inverse of the Jacobian of transformation
  // and the matrix vector product has to be calculated. This follows.

  std::array< std::vector<tk::real>, 3 > dBdx;
  dBdx[0].resize( ndof, 0 );
  dBdx[1].resize( ndof, 0 );
  dBdx[2].resize( ndof, 0 );

  auto db2dxi1 = 2.0;
  auto db2dxi2 = 1.0;
  auto db2dxi3 = 1.0;

  auto db3dxi1 = 0.0;
  auto db3dxi2 = 3.0;
  auto db3dxi3 = 1.0;

  auto db4dxi1 = 0.0;
  auto db4dxi2 = 0.0;
  auto db4dxi3 = 4.0;

  dBdx[0][1] =  db2dxi1 * jacInv[0][0]
              + db2dxi2 * jacInv[1][0]
              + db2dxi3 * jacInv[2][0];

  dBdx[1][1] =  db2dxi1 * jacInv[0][1]
              + db2dxi2 * jacInv[1][1]
              + db2dxi3 * jacInv[2][1];

  dBdx[2][1] =  db2dxi1 * jacInv[0][2]
              + db2dxi2 * jacInv[1][2]
              + db2dxi3 * jacInv[2][2];

  dBdx[0][2] =  db3dxi1 * jacInv[0][0]
              + db3dxi2 * jacInv[1][0]
              + db3dxi3 * jacInv[2][0];

  dBdx[1][2] =  db3dxi1 * jacInv[0][1]
              + db3dxi2 * jacInv[1][1]
              + db3dxi3 * jacInv[2][1];

  dBdx[2][2] =  db3dxi1 * jacInv[0][2]
              + db3dxi2 * jacInv[1][2]
              + db3dxi3 * jacInv[2][2];

  dBdx[0][3] =  db4dxi1 * jacInv[0][0]
              + db4dxi2 * jacInv[1][0]
              + db4dxi3 * jacInv[2][0];

  dBdx[1][3] =  db4dxi1 * jacInv[0][1]
              + db4dxi2 * jacInv[1][1]
              + db4dxi3 * jacInv[2][1];

  dBdx[2][3] =  db4dxi1 * jacInv[0][2]
              + db4dxi2 * jacInv[1][2]
              + db4dxi3 * jacInv[2][2];

  return dBdx;
}

void
tk::eval_dBdx_p2( const std::size_t igp,
                  const std::array< std::vector< tk::real >, 3 >& coordgp,
                  const std::array< std::array< tk::real, 3 >, 3 >& jacInv,
                  std::array< std::vector<tk::real>, 3 >& dBdx )
// *****************************************************************************
//  Compute the derivatives of basis function for DG(P2)
//! \param[in] igp Index of quadrature points
//! \param[in] coordgp Gauss point coordinates for tetrahedron element
//! \param[in] jacInv Array of the inverse of Jacobian
//! \param[in,out] dBdx Array of the derivatives of basis function
// *****************************************************************************
{
  auto db5dxi1 = 12.0 * coordgp[0][igp] + 6.0 * coordgp[1][igp]
               +  6.0 * coordgp[2][igp] - 6.0;
  auto db5dxi2 =  6.0 * coordgp[0][igp] + 2.0 * coordgp[1][igp]
               +  2.0 * coordgp[2][igp] - 2.0;
  auto db5dxi3 =  6.0 * coordgp[0][igp] + 2.0 * coordgp[1][igp]
               +  2.0 * coordgp[2][igp] - 2.0;

  auto db6dxi1 = 10.0 * coordgp[1][igp] +  2.0 * coordgp[2][igp] - 2.0;
  auto db6dxi2 = 10.0 * coordgp[0][igp] + 10.0 * coordgp[1][igp]
               +  6.0 * coordgp[2][igp] - 6.0;
  auto db6dxi3 =  2.0 * coordgp[0][igp] +  6.0 * coordgp[1][igp]
               +  2.0 * coordgp[2][igp] - 2.0;

  auto db7dxi1 = 12.0 * coordgp[2][igp] - 2.0;
  auto db7dxi2 =  6.0 * coordgp[2][igp] - 1.0;
  auto db7dxi3 = 12.0 * coordgp[0][igp] + 6.0 * coordgp[1][igp]
               + 12.0 * coordgp[2][igp] - 7.0;

  auto db8dxi1 =  0;
  auto db8dxi2 = 20.0 * coordgp[1][igp] + 8.0 * coordgp[2][igp] - 8.0;
  auto db8dxi3 =  8.0 * coordgp[1][igp] + 2.0 * coordgp[2][igp] - 2.0;

  auto db9dxi1 =  0;
  auto db9dxi2 = 18.0 * coordgp[2][igp] -  3.0;
  auto db9dxi3 = 18.0 * coordgp[1][igp] + 12.0 * coordgp[2][igp] - 7.0;

  auto db10dxi1 =  0;
  auto db10dxi2 =  0;
  auto db10dxi3 = 30.0 * coordgp[2][igp] - 10.0;

  dBdx[0][4] =  db5dxi1 * jacInv[0][0]
              + db5dxi2 * jacInv[1][0]
              + db5dxi3 * jacInv[2][0];

  dBdx[1][4] =  db5dxi1 * jacInv[0][1]
              + db5dxi2 * jacInv[1][1]
              + db5dxi3 * jacInv[2][1];

  dBdx[2][4] =  db5dxi1 * jacInv[0][2]
              + db5dxi2 * jacInv[1][2]
              + db5dxi3 * jacInv[2][2];

  dBdx[0][5] =  db6dxi1 * jacInv[0][0]
              + db6dxi2 * jacInv[1][0]
              + db6dxi3 * jacInv[2][0];

  dBdx[1][5] =  db6dxi1 * jacInv[0][1]
              + db6dxi2 * jacInv[1][1]
              + db6dxi3 * jacInv[2][1];

  dBdx[2][5] =  db6dxi1 * jacInv[0][2]
              + db6dxi2 * jacInv[1][2]
              + db6dxi3 * jacInv[2][2];

  dBdx[0][6] =  db7dxi1 * jacInv[0][0]
              + db7dxi2 * jacInv[1][0]
              + db7dxi3 * jacInv[2][0];

  dBdx[1][6] =  db7dxi1 * jacInv[0][1]
              + db7dxi2 * jacInv[1][1]
              + db7dxi3 * jacInv[2][1];

  dBdx[2][6] =  db7dxi1 * jacInv[0][2]
              + db7dxi2 * jacInv[1][2]
              + db7dxi3 * jacInv[2][2];

  dBdx[0][7] =  db8dxi1 * jacInv[0][0]
              + db8dxi2 * jacInv[1][0]
              + db8dxi3 * jacInv[2][0];

  dBdx[1][7] =  db8dxi1 * jacInv[0][1]
              + db8dxi2 * jacInv[1][1]
              + db8dxi3 * jacInv[2][1];

  dBdx[2][7] =  db8dxi1 * jacInv[0][2]
              + db8dxi2 * jacInv[1][2]
              + db8dxi3 * jacInv[2][2];

  dBdx[0][8] =  db9dxi1 * jacInv[0][0]
              + db9dxi2 * jacInv[1][0]
              + db9dxi3 * jacInv[2][0];

  dBdx[1][8] =  db9dxi1 * jacInv[0][1]
              + db9dxi2 * jacInv[1][1]
              + db9dxi3 * jacInv[2][1];

  dBdx[2][8] =  db9dxi1 * jacInv[0][2]
              + db9dxi2 * jacInv[1][2]
              + db9dxi3 * jacInv[2][2];

  dBdx[0][9] =  db10dxi1 * jacInv[0][0]
              + db10dxi2 * jacInv[1][0]
              + db10dxi3 * jacInv[2][0];

  dBdx[1][9] =  db10dxi1 * jacInv[0][1]
              + db10dxi2 * jacInv[1][1]
              + db10dxi3 * jacInv[2][1];

  dBdx[2][9] =  db10dxi1 * jacInv[0][2]
              + db10dxi2 * jacInv[1][2]
              + db10dxi3 * jacInv[2][2];
}

std::vector< tk::real >
tk::eval_basis( const std::size_t ndof,
                const tk::real xi,
                const tk::real eta,
                const tk::real zeta )
// *****************************************************************************
//  Compute the Dubiner basis functions
//! \param[in] ndof Number of degrees of freedom
//! \param[in] xi,eta,zeta Coordinates for quadrature points in reference space
//! \return Vector of basis functions
// *****************************************************************************
{
  // Array of basis functions
  std::vector< tk::real > B( ndof, 1.0 );

  if ( ndof > 1 )           // DG(P1)
  {
    B[1] = 2.0 * xi + eta + zeta - 1.0;
    B[2] = 3.0 * eta + zeta - 1.0;
    B[3] = 4.0 * zeta - 1.0;

    if( ndof > 4 )         // DG(P2)
    {
      B[4] =  6.0 * xi * xi + eta * eta + zeta * zeta
            + 6.0 * xi * eta + 6.0 * xi * zeta + 2.0 * eta * zeta
            - 6.0 * xi - 2.0 * eta - 2.0 * zeta + 1.0;
      B[5] =  5.0 * eta * eta + zeta * zeta
            + 10.0 * xi * eta + 2.0 * xi * zeta + 6.0 * eta * zeta
            - 2.0 * xi - 6.0 * eta - 2.0 * zeta + 1.0;
      B[6] =  6.0 * zeta * zeta + 12.0 * xi * zeta + 6.0 * eta * zeta - 2.0 * xi
            - eta - 7.0 * zeta + 1.0;
      B[7] =  10.0 * eta * eta + zeta * zeta + 8.0 * eta * zeta
            - 8.0 * eta - 2.0 * zeta + 1.0;
      B[8] =  6.0 * zeta * zeta + 18.0 * eta * zeta - 3.0 * eta - 7.0 * zeta
            + 1.0;
      B[9] =  15.0 * zeta * zeta - 10.0 * zeta + 1.0;
    }
  }

  return B;
}

std::vector< tk::real >
tk::eval_state ( ncomp_t ncomp,
                 ncomp_t offset,
                 const std::size_t ndof,
                 const std::size_t ndof_el,
                 const std::size_t e,
                 const Fields& U,
                 const std::vector< tk::real >& B )
// *****************************************************************************
//  Compute the state variables for the tetrahedron element
//! \param[in] ncomp Number of scalar components in this PDE system
//! \param[in] offset Offset this PDE system operates from
//! \param[in] ndof Maximum number of degrees of freedom
//! \param[in] ndof_el Number of degrees of freedom for the local element
//! \param[in] e Index for the tetrahedron element
//! \param[in] U Solution vector at recent time step
//! \param[in] B Vector of basis functions
//! \return Vector of state variable for tetrahedron element
// *****************************************************************************
{
  // This is commented for now because that when p0/p1 adaptive with limiter
  // applied, the size of basis will be 10. However, ndof_el will be 4 which
  // leads to a size mismatch in limiter function.
  //Assert( B.size() == ndof_el, "Size mismatch" );

  if (U.empty()) return {};

  // Array of state variable for tetrahedron element
  std::vector< tk::real > state( ncomp, 0.0 );

  for (ncomp_t c=0; c<ncomp; ++c)
  {
    auto mark = c*ndof;
    state[c] = U( e, mark, offset );

    if(ndof_el > 1)        //DG(P1)
    {
      state[c] += U( e, mark+1, offset ) * B[1]
                + U( e, mark+2, offset ) * B[2]
                + U( e, mark+3, offset ) * B[3];
    }

    if(ndof_el > 4)        //DG(P2)
    {
      state[c] += U( e, mark+4, offset ) * B[4]
                + U( e, mark+5, offset ) * B[5]
                + U( e, mark+6, offset ) * B[6]
                + U( e, mark+7, offset ) * B[7]
                + U( e, mark+8, offset ) * B[8]
                + U( e, mark+9, offset ) * B[9];
    }
  }

  return state;
}

void
tk::evaldBdx_p2(  const std::vector< tk::real >& coord,
                  const std::array< std::array< tk::real, 3 >, 3 >& jacInv,
                  std::array< std::vector<tk::real>, 3 >& dBdx )
// *****************************************************************************
//  Compute the derivatives of basis function in physical domain
//! \param[in] coord point coordinates for tetrahedron element
//! \param[in] jacInv Array of the inverse of Jacobian
//! \param[in,out] dBdx Array of the derivatives of basis function
// *****************************************************************************
{
  auto db5dxi1 = 12.0 * coord[0] + 6.0 * coord[1]
               +  6.0 * coord[2] - 6.0;
  auto db5dxi2 =  6.0 * coord[0] + 2.0 * coord[1]
               +  2.0 * coord[2] - 2.0;
  auto db5dxi3 =  6.0 * coord[0] + 2.0 * coord[1]
               +  2.0 * coord[2] - 2.0;

  auto db6dxi1 = 10.0 * coord[1] +  2.0 * coord[2] - 2.0;
  auto db6dxi2 = 10.0 * coord[0] + 10.0 * coord[1]
               +  6.0 * coord[2] - 6.0;
  auto db6dxi3 =  2.0 * coord[0] +  6.0 * coord[1]
               +  2.0 * coord[2] - 2.0;

  auto db7dxi1 = 12.0 * coord[2] - 2.0;
  auto db7dxi2 =  6.0 * coord[2] - 1.0;
  auto db7dxi3 = 12.0 * coord[0] + 6.0 * coord[1]
               + 12.0 * coord[2] - 7.0;

  auto db8dxi1 =  0;
  auto db8dxi2 = 20.0 * coord[1] + 8.0 * coord[2] - 8.0;
  auto db8dxi3 =  8.0 * coord[1] + 2.0 * coord[2] - 2.0;

  auto db9dxi1 =  0;
  auto db9dxi2 = 18.0 * coord[2] -  3.0;
  auto db9dxi3 = 18.0 * coord[1] + 12.0 * coord[2] - 7.0;

  auto db10dxi1 =  0;
  auto db10dxi2 =  0;
  auto db10dxi3 = 30.0 * coord[2] - 10.0;

  dBdx[0][4] = db5dxi1 * jacInv[0][0]
             + db5dxi2 * jacInv[1][0]
             + db5dxi3 * jacInv[2][0];

  dBdx[1][4] = db5dxi1 * jacInv[0][1]
             + db5dxi2 * jacInv[1][1]
             + db5dxi3 * jacInv[2][1];

  dBdx[2][4] = db5dxi1 * jacInv[0][2]
             + db5dxi2 * jacInv[1][2]
             + db5dxi3 * jacInv[2][2];

  dBdx[0][5] = db6dxi1 * jacInv[0][0]
             + db6dxi2 * jacInv[1][0]
             + db6dxi3 * jacInv[2][0];

  dBdx[1][5] = db6dxi1 * jacInv[0][1]
             + db6dxi2 * jacInv[1][1]
             + db6dxi3 * jacInv[2][1];

  dBdx[2][5] = db6dxi1 * jacInv[0][2]
             + db6dxi2 * jacInv[1][2]
             + db6dxi3 * jacInv[2][2];

  dBdx[0][6] = db7dxi1 * jacInv[0][0]
             + db7dxi2 * jacInv[1][0]
             + db7dxi3 * jacInv[2][0];

  dBdx[1][6] = db7dxi1 * jacInv[0][1]
             + db7dxi2 * jacInv[1][1]
             + db7dxi3 * jacInv[2][1];

  dBdx[2][6] = db7dxi1 * jacInv[0][2]
             + db7dxi2 * jacInv[1][2]
             + db7dxi3 * jacInv[2][2];

  dBdx[0][7] = db8dxi1 * jacInv[0][0]
             + db8dxi2 * jacInv[1][0]
             + db8dxi3 * jacInv[2][0];

  dBdx[1][7] = db8dxi1 * jacInv[0][1]
             + db8dxi2 * jacInv[1][1]
             + db8dxi3 * jacInv[2][1];

  dBdx[2][7] = db8dxi1 * jacInv[0][2]
             + db8dxi2 * jacInv[1][2]
             + db8dxi3 * jacInv[2][2];

  dBdx[0][8] = db9dxi1 * jacInv[0][0]
             + db9dxi2 * jacInv[1][0]
             + db9dxi3 * jacInv[2][0];

  dBdx[1][8] = db9dxi1 * jacInv[0][1]
             + db9dxi2 * jacInv[1][1]
             + db9dxi3 * jacInv[2][1];

  dBdx[2][8] = db9dxi1 * jacInv[0][2]
             + db9dxi2 * jacInv[1][2]
             + db9dxi3 * jacInv[2][2];

  dBdx[0][9] = db10dxi1 * jacInv[0][0]
             + db10dxi2 * jacInv[1][0]
             + db10dxi3 * jacInv[2][0];

  dBdx[1][9] = db10dxi1 * jacInv[0][1]
             + db10dxi2 * jacInv[1][1]
             + db10dxi3 * jacInv[2][1];

  dBdx[2][9] = db10dxi1 * jacInv[0][2]
             + db10dxi2 * jacInv[1][2]
             + db10dxi3 * jacInv[2][2];
}

void tk::TransformBasis( ncomp_t ncomp,
                         ncomp_t offset,
                         const std::size_t e,
                         const std::size_t ndof,
                         const tk::Fields& U,
                         const std::vector< std::size_t >& inpoel,
                         const tk::UnsMesh::Coords& coord,
                         std::vector< std::vector< tk::real > >& unk)
// *****************************************************************************
//  Transform the solution with Dubiner basis to the solution with Taylor basis
//! \param[in] ncomp Number of scalar components in this PDE system
//! \param[in] offset Index for equation systems
//! \param[in] e Id of element whose solution is to be limited
//! \param[in] ndof Maximum number of degrees of freedom
//! \param[in] U High-order solution vector with Dubiner basis
//! \param[in] inpoel Element connectivity
//! \param[in] coord Array of nodal coordinates
//! \param[in] unk High-order solution vector with Taylor basis
// *****************************************************************************
{
  const auto& cx = coord[0];
  const auto& cy = coord[1];
  const auto& cz = coord[2];

  std::vector< tk::real > center{0.25, 0.25, 0.25};

  // Evaluate the cell center solution
  for(ncomp_t icomp = 0; icomp < ncomp; icomp++)
  {
    auto mark = icomp * ndof;
    unk[icomp][0] = U(e, mark, offset);
  }

  // Evaluate the first order derivative
  std::array< std::array< tk::real, 3>, 4 > coordel {{
    {{ cx[ inpoel[4*e  ] ], cy[ inpoel[4*e  ] ], cz[ inpoel[4*e  ] ] }},
    {{ cx[ inpoel[4*e+1] ], cy[ inpoel[4*e+1] ], cz[ inpoel[4*e+1] ] }},
    {{ cx[ inpoel[4*e+2] ], cy[ inpoel[4*e+2] ], cz[ inpoel[4*e+2] ] }},
    {{ cx[ inpoel[4*e+3] ], cy[ inpoel[4*e+3] ], cz[ inpoel[4*e+3] ] }}
  }};

  auto jacInv =
              tk::inverseJacobian( coordel[0], coordel[1], coordel[2], coordel[3] );

  // Compute the derivatives of basis function for DG(P1)
  auto dBdx = tk::eval_dBdx_p1( ndof, jacInv );

  if(ndof > 4)
    tk::evaldBdx_p2(center, jacInv, dBdx);

  for(ncomp_t icomp = 0; icomp < ncomp; icomp++)
  {
    auto mark = icomp * ndof; 
    for(std::size_t idir = 0; idir < 3; idir++)
    {
      unk[icomp][idir+1] = 0;
      for(std::size_t idof = 1; idof < ndof; idof++)
        unk[icomp][idir+1] += U(e, mark+idof, offset) * dBdx[idir][idof];
    }
  }

  // Evaluate the second order derivative if DGP2 is applied
  // The basic idea of the computation follows
  //    d2Udx2 = /sum u_i * (d2B_i/dx2)
  // where d2B_i/dx2 = d( dB_i/dxi * dxi/dx ) / dxi * dxi/dx
  if(ndof > 4)
  {
    std::vector< std::vector< tk::real > > dB2dxi2
    { { 12.0,  0.0,  0.0,  0.0,  0.0,  0.0 },
      {  2.0, 10.0,  0.0, 20.0,  0.0,  0.0 },
      {  2.0,  2.0, 12.0,  2.0, 12.0, 30.0 },
      {  6.0, 10.0,  0.0,  0.0,  0.0,  0.0 },
      {  6.0,  2.0, 12.0,  0.0,  0.0,  0.0 },
      {  2.0,  6.0,  6.0,  8.0, 18.0,  0.0 } };

    std::vector< std::vector< tk::real > > d2Bdx2;
    d2Bdx2.resize(6, std::vector< tk::real>(6,0.0) );
    for(std::size_t ibasis = 0; ibasis < 6; ibasis++)
    {
      for(std::size_t idir = 0; idir < 3; idir++)
        d2Bdx2[idir][ibasis] +=
            dB2dxi2[0][ibasis] * jacInv[0][idir] * jacInv[0][idir]
          + dB2dxi2[1][ibasis] * jacInv[1][idir] * jacInv[1][idir]
          + dB2dxi2[2][ibasis] * jacInv[2][idir] * jacInv[2][idir]
          + 2.0 *( dB2dxi2[3][ibasis] * jacInv[0][idir] * jacInv[1][idir]
                 + dB2dxi2[4][ibasis] * jacInv[0][idir] * jacInv[2][idir]
                 + dB2dxi2[5][ibasis] * jacInv[1][idir] * jacInv[2][idir] );

      d2Bdx2[3][ibasis] +=
          dB2dxi2[0][ibasis] * jacInv[0][0] * jacInv[0][1]
        + dB2dxi2[1][ibasis] * jacInv[1][0] * jacInv[1][1]
        + dB2dxi2[2][ibasis] * jacInv[2][0] * jacInv[2][1]
        + dB2dxi2[3][ibasis]
        * (jacInv[0][0] * jacInv[1][1] + jacInv[1][0] * jacInv[0][1])
        + dB2dxi2[4][ibasis]
        * (jacInv[0][0] * jacInv[2][1] + jacInv[2][0] * jacInv[0][1])
        + dB2dxi2[5][ibasis]
        * (jacInv[1][0] * jacInv[2][1] + jacInv[2][0] * jacInv[1][1]);

      d2Bdx2[4][ibasis] +=
          dB2dxi2[0][ibasis] * jacInv[0][0] * jacInv[0][2]
        + dB2dxi2[1][ibasis] * jacInv[1][0] * jacInv[1][2]
        + dB2dxi2[2][ibasis] * jacInv[2][0] * jacInv[2][2]
        + dB2dxi2[3][ibasis]
        * (jacInv[0][0] * jacInv[1][2] + jacInv[1][0] * jacInv[0][2])
        + dB2dxi2[4][ibasis]
        * (jacInv[0][0] * jacInv[2][2] + jacInv[2][0] * jacInv[0][2])
        + dB2dxi2[5][ibasis]
        * (jacInv[1][0] * jacInv[2][2] + jacInv[2][0] * jacInv[1][2]);

      d2Bdx2[5][ibasis] +=
          dB2dxi2[0][ibasis] * jacInv[0][1] * jacInv[0][2]
        + dB2dxi2[1][ibasis] * jacInv[1][1] * jacInv[1][2]
        + dB2dxi2[2][ibasis] * jacInv[2][1] * jacInv[2][2]
        + dB2dxi2[3][ibasis]
        * (jacInv[0][1] * jacInv[1][2] + jacInv[1][1] * jacInv[0][2])
        + dB2dxi2[4][ibasis]
        * (jacInv[0][1] * jacInv[2][2] + jacInv[2][1] * jacInv[0][2])
        + dB2dxi2[5][ibasis]
        * (jacInv[1][1] * jacInv[2][2] + jacInv[2][1] * jacInv[1][2]);
    }

    for(ncomp_t icomp = 0; icomp < ncomp; icomp++)
    {
      auto mark = icomp * ndof;
      for(std::size_t idir = 0; idir < 6; idir++)
      {
        unk[icomp][idir+4] = 0;
        for(std::size_t ibasis = 0; ibasis < 6; ibasis++)
          unk[icomp][idir+4] += U(e, mark+4+ibasis, offset) * d2Bdx2[idir][ibasis];
      }
    }
  }
}

void tk::InverseBasis( ncomp_t ncomp,
                       ncomp_t offset,
                       std::size_t e,
                       std::size_t ndof,
                       const std::vector< std::size_t >& inpoel,
                       const tk::UnsMesh::Coords& coord,
                       const tk::Fields& geoElem,
                       tk::Fields& U,
                       std::vector< std::vector< tk::real > >& unk )
// *****************************************************************************
//  Convert the solution with Taylor basis to the solution with Dubiner basis by
//    projection method
//! \param[in] ncomp Number of scalar components in this PDE system
//! \param[in] e Id of element whose solution is to be limited
//! \param[in] ndof Maximum number of degrees of freedom
//! \param[in] inpoel Element connectivity
//! \param[in] coord Array of nodal coordinates
//! \param[in] U High-order solution vector with Dubiner basis
//! \param[in] unk High-order solution vector with Taylor basis
// *****************************************************************************
{
  // The diagonal of mass matrix
  std::vector< tk::real > L(ndof, 0.0);

  tk::real vol = 1.0 / 6.0;

  L[0] = vol;

  L[1] = vol / 10.0;
  L[2] = vol * 3.0/10.0;
  L[3] = vol * 3.0/5.0;

  if(ndof > 4)
  {
    L[4] = vol / 35.0;
    L[5] = vol / 21.0;
    L[6] = vol / 14.0;
    L[7] = vol / 7.0;
    L[8] = vol * 3.0/14.0;
    L[9] = vol * 3.0/7.0;
  }

  // Coordinates of the centroid in physical domain
  std::array< tk::real, 3 > x_c{geoElem(e,1,0), geoElem(e,2,0), geoElem(e,3,0)};

  const auto& cx = coord[0];
  const auto& cy = coord[1];
  const auto& cz = coord[2];

  std::array< std::array< tk::real, 3>, 4 > coordel {{
    {{ cx[ inpoel[4*e  ] ], cy[ inpoel[4*e  ] ], cz[ inpoel[4*e  ] ] }},
    {{ cx[ inpoel[4*e+1] ], cy[ inpoel[4*e+1] ], cz[ inpoel[4*e+1] ] }},
    {{ cx[ inpoel[4*e+2] ], cy[ inpoel[4*e+2] ], cz[ inpoel[4*e+2] ] }},
    {{ cx[ inpoel[4*e+3] ], cy[ inpoel[4*e+3] ], cz[ inpoel[4*e+3] ] }}
  }};

  // Number of quadrature points for volume integration
  auto ng = tk::NGvol(ndof);

  // arrays for quadrature points
  std::array< std::vector< tk::real >, 3 > coordgp;
  std::vector< tk::real > wgp;

  coordgp[0].resize( ng );
  coordgp[1].resize( ng );
  coordgp[2].resize( ng );
  wgp.resize( ng );

  // get quadrature point weights and coordinates for triangle
  tk::GaussQuadratureTet( ng, coordgp, wgp );

  // right hand side vector
  std::vector< tk::real > R( ncomp*ndof, 0.0 );

  // Gaussian quadrature
  for (std::size_t igp=0; igp<ng; ++igp)
  {
    auto wt = wgp[igp] * vol;

    auto gp = tk::eval_gp( igp, coordel, coordgp );

    auto B_taylor = eval_TaylorBasis( ndof, gp, x_c, coordel);

    // Compute high order solution at gauss point
    std::vector< tk::real > state( ncomp, 0.0 );
    for (ncomp_t c=0; c<ncomp; ++c)
    {
      state[c] = unk[c][0];
      state[c] += unk[c][1] * B_taylor[1]
                + unk[c][2] * B_taylor[2]
                + unk[c][3] * B_taylor[3];

      if(ndof > 4)
        state[c] += unk[c][4] * B_taylor[4] + unk[c][5] * B_taylor[5]
                  + unk[c][6] * B_taylor[6] + unk[c][7] * B_taylor[7]
                  + unk[c][8] * B_taylor[8] + unk[c][9] * B_taylor[9];
    }

    auto B = tk::eval_basis( ndof, coordgp[0][igp], coordgp[1][igp], coordgp[2][igp] );

    for (ncomp_t c=0; c<ncomp; ++c)
    {
      auto mark = c*ndof;
      R[mark] += wt * state[c];

      if(ndof > 1)
      {
        R[mark+1] += wt * state[c] * B[1];
        R[mark+2] += wt * state[c] * B[2];
        R[mark+3] += wt * state[c] * B[3];

        if(ndof > 4)
        {
          R[mark+4] += wt * state[c] * B[4];
          R[mark+5] += wt * state[c] * B[5];
          R[mark+6] += wt * state[c] * B[6];
          R[mark+7] += wt * state[c] * B[7];
          R[mark+8] += wt * state[c] * B[8];
          R[mark+9] += wt * state[c] * B[9];
        }
      }
    }
  }

  for (ncomp_t c=0; c<ncomp; ++c)
  {
    auto mark = c*ndof;
    U(e, mark, offset) = R[mark] / L[0];

    if(ndof > 1)
    {
      U(e, mark+1, offset) = R[mark+1] / L[1];
      U(e, mark+2, offset) = R[mark+2] / L[2];
      U(e, mark+3, offset) = R[mark+3] / L[3];

      if(ndof > 4)
      {
        U(e, mark+4, offset) = R[mark+4] / L[4];
        U(e, mark+5, offset) = R[mark+5] / L[5];
        U(e, mark+6, offset) = R[mark+6] / L[6];
        U(e, mark+7, offset) = R[mark+7] / L[7];
        U(e, mark+8, offset) = R[mark+8] / L[8];
        U(e, mark+9, offset) = R[mark+9] / L[9];
      }
    }
  }
}

std::vector< tk::real >
tk::eval_TaylorBasis( const std::size_t ndof,
                      const std::array< tk::real, 3 >& x,
                      const std::array< tk::real, 3 >& x_c,
                      const std::array< std::array< tk::real, 3>, 4 >& coordel )
// *****************************************************************************
//  Evaluate the Taylor basis at points
//! \param[in] ndof Maximum number of degrees of freedom
//! \param[in] x Nodal coordinates
//! \param[in] x_c Coordinates of the centroid
//! \param[in] coordel Array of nodal coordinates for the tetrahedron
// *****************************************************************************
{
  std::vector< tk::real > avg( 6, 0.0 );
  if(ndof > 4)
  {
    auto ng = tk::NGvol(ndof);

    std::array< std::vector< tk::real >, 3 > coordgp;
    std::vector< tk::real > wgp;

    coordgp[0].resize( ng );
    coordgp[1].resize( ng );
    coordgp[2].resize( ng );
    wgp.resize( ng );

    tk::GaussQuadratureTet( ng, coordgp, wgp );

    for (std::size_t igp=0; igp<ng; ++igp)
    {
      // Compute the coordinates of quadrature point at physical domain
      auto gp = tk::eval_gp( igp, coordel, coordgp );

      avg[0] += wgp[igp] * (gp[0] - x_c[0]) * (gp[0] - x_c[0]) * 0.5;
      avg[1] += wgp[igp] * (gp[1] - x_c[1]) * (gp[1] - x_c[1]) * 0.5;
      avg[2] += wgp[igp] * (gp[2] - x_c[2]) * (gp[2] - x_c[2]) * 0.5;
      avg[3] += wgp[igp] * (gp[0] - x_c[0]) * (gp[1] - x_c[1]);
      avg[4] += wgp[igp] * (gp[0] - x_c[0]) * (gp[2] - x_c[2]);
      avg[5] += wgp[igp] * (gp[1] - x_c[1]) * (gp[2] - x_c[2]);
    }
  }

  std::vector< tk::real > B( ndof, 1.0 );

  B[1] = x[0] - x_c[0];
  B[2] = x[1] - x_c[1];
  B[3] = x[2] - x_c[2];

  if( ndof > 4 )
  {
    B[4] = B[1] * B[1] * 0.5 - avg[0];
    B[5] = B[2] * B[2] * 0.5 - avg[1];
    B[6] = B[3] * B[3] * 0.5 - avg[2];
    B[7] = B[1] * B[2] - avg[3];
    B[8] = B[1] * B[3] - avg[4];
    B[9] = B[2] * B[3] - avg[5];
  }

  return B;
}
