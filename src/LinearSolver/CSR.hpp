// *****************************************************************************
/*!
  \file      src/LinearSolver/CSR.hpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019-2020 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Compressed sparse row (CSR) storage for a sparse matrix
  \details   Compressed sparse row (CSR) storage for a sparse matrix.
*/
// *****************************************************************************
#ifndef CSR_h
#define CSR_h

#include <vector>

#include "Types.hpp"

namespace tk {

//! Compressed sparse row (CSR) storage for a sparse matrix
class CSR {

  public:
    //! \brief Constructor: Create a CSR matrix for a size x size sparse
    //!   symmetric matrix with DOF degrees of freedom, storing only the upper
    //!   triangular part.
    //! \param[in] DOF Number of scalar components (degrees of freedom)
    //! \param[in] size Number of scalar components (degrees of freedom)
    //! \param[in] psup Points surrounding points of mesh graph, see tk::genPsup
    explicit CSR( std::size_t DOF,
                  std::size_t size,
                  const std::pair< std::vector< std::size_t >,
                                   std::vector< std::size_t > >& psup );

    //! Return const reference to sparse matrix entry at a position
    const tk::real&
    operator()( std::size_t row, std::size_t col, std::size_t pos=0 ) const;

    //! Return non-const reference to sparse matrix entry at a position
    //! \see "Avoid Duplication in const and Non-const Member Function," and
    //!   "Use const whenever possible," Scott Meyers, Effective C++, 3d ed.
    tk::real&
    operator()( std::size_t row, std::size_t col, std::size_t pos=0 ) {
      return const_cast< tk::real& >(
               static_cast< const CSR& >( *this ).operator()( row, col, pos ) );
    }

  private:
    std::size_t dof;                    //!< Number of degrees of freedom
    std::vector< std::size_t > rnz;     //!< Number of nonzeros of each row
    std::vector< std::size_t > ia;      //!< Row pointers
    std::vector< std::size_t > ja;      //!< Column indices
    std::vector< tk::real > a;          //!< Nonzero values
};

} // tk::

#endif // Around_h
