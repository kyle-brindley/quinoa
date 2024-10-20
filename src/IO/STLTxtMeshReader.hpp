// *****************************************************************************
/*!
  \file      src/IO/STLTxtMeshReader.hpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019-2021 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     ASCII STL (STereoLithography) reader class declaration
  \details   ASCII STL (STereoLithographu) reader class declaration.
*/
// *****************************************************************************
#ifndef STLTxtMeshReader_h
#define STLTxtMeshReader_h

#include <iostream>
#include <stddef.h>
#include <string>

#include "Types.hpp"
#include "Reader.hpp"
#include "Exception.hpp"

namespace tk {

class STLMesh;

//! \brief STLTxtMeshReader : tk::Reader
//! \details Mesh reader class facilitating reading a mesh from a file in
//!   ASCII STL format.
class STLTxtMeshReader : public Reader {

  public:
    //! Constructor
    explicit STLTxtMeshReader( const std::string& filename, STLMesh& mesh );

    //! Read ASCII STL mesh
    void readMesh();

  private:
    //! \brief ASCII STL keyword with operator>> redefined to do error checking
    //!    without contaminating client-code
    struct STLKeyword {
      std::string read;                 //!< Keyword read in from input
      const std::string correct;        //!< Keyword that should be read in

      //! Initializer constructor
      explicit STLKeyword( const std::string& corr ) noexcept :
        read(), correct(corr) {}

      //! Operator >> for reading a keyword and hande error
      friend std::ifstream& operator>> (std::ifstream& is, STLKeyword& kw) {
        is >> kw.read;
        ErrChk( kw.read == kw.correct,
                "Corruption in ASCII STL file while parsing keyword '" +
                kw.read + "', should be '" + kw.correct + "'" );
        return is;
      }
    };

    //! Read (or count vertices in) ASCII STL mesh
    std::size_t readFacets( const bool store,
                            tk::real* const x = nullptr,
                            tk::real* const y = nullptr,
                            tk::real* const z = nullptr );

    const bool STORE = true;                 //!< Indicator to store facets
    const bool COUNT = false;                //!< Indicator to only count facets
    STLMesh& m_mesh;                         //!< Mesh
};

} // tk::

#endif // STLTxtMeshReader_h
