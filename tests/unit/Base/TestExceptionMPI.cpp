// *****************************************************************************
/*!
  \file      tests/unit/Base/TestExceptionMPI.cpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019-2021 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Unit tests for Base/TestExceptionMPI.hpp
  \details   Unit tests for Base/TestExceptionMPI.hpp
*/
// *****************************************************************************

#include "NoWarning/tut.hpp"

#include "TUTConfig.hpp"
#include "ExceptionMPI.hpp"

#ifndef DOXYGEN_GENERATING_OUTPUT

namespace tut {

//! All tests in group inherited from this base
struct ExceptionMPI_common {};

//! Test group shortcuts
using ExceptionMPI_group =
  test_group< ExceptionMPI_common, MAX_TESTS_IN_GROUP >;
using ExceptionMPI_object = ExceptionMPI_group::object;

//! Define test group
//! \note Those test groups whose name contains "MPI" will be started as
//!    MPI tests (from a Charm++ nodegroup)
static ExceptionMPI_group ExceptionMPI( "Base/ExceptionMPI" );

//! Test definitions for group

//! Test that AssertMPI macro throws if condition is false on all ranks
template<> template<>
void ExceptionMPI_object::test< 1 >() {
  set_test_name( "AssertMPI macro throws all false" );

  try {
    AssertMPI( 0 == 1, "msg" );
    #ifndef NDEBUG
    fail( "should throw exception in DEBUG mode" );
    #endif
  }
  catch ( tk::Exception& ) {
    // exception thrown in DEBUG mode, test ok
    // Assert skipped in RELEASE mode, test ok
  }
}

//! Test that AssertMPI macro does not throw if condition is true on all ranks
template<> template<>
void ExceptionMPI_object::test< 2 >() {
  set_test_name( "AssertMPI macro doesn't throw all true" );

  try {
    // cppcheck-suppress duplicateExpression
    AssertMPI( 1 == 1, "msg" );
  }
  catch ( tk::Exception& ) {
    fail( "should not throw exception" );
  }
}

// The ErrChkMPI macro generates a warning that is disabled below. It would be
// great if this can be done inside that macro in Base/ExceptionMPI.h.
#if defined(__clang__)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wold-style-cast"
#endif

//! Test that ErrChkMPI macro throws if condition is false on all ranks
template<> template<>
void ExceptionMPI_object::test< 3 >() {
  set_test_name( "ErrChkMPI macro throws all false" );

  try {
    ErrChkMPI( 0 == 1, "msg" )
    fail( "should throw excecption" );
  }
  catch ( tk::Exception& ) {
    // exception thrown, test ok
  }
}

//! Test that ErrChkMPI macro does not throw if condition is true on all ranks
template<> template<>
void ExceptionMPI_object::test< 4 >() {
  set_test_name( "ErrChkMPI macro doesn't throw all true" );

  try {
    ErrChkMPI( 0 != 1, "msg" )
  }
  catch ( tk::Exception& ) {
    fail( "should not throw excecption" );
  }
}

//! Test that ErrChkMPI macro throws if condition is false on the 0th rank
template<> template<>
void ExceptionMPI_object::test< 5 >() {
  set_test_name( "ErrChkMPI macro throws 0th false" );

  try {
    int peid;
    MPI_Comm_rank( MPI_COMM_WORLD, &peid );
    // cppcheck-suppress duplicateExpression
    ErrChkMPI( peid == 0 ? 0 == 1 : 1 == 1, "msg" )
    fail( "should throw exception" );
  }
  catch ( tk::Exception& ) {
    // exception thrown, test ok
  }
}

//! Test that ErrChkMPI macro throws if condition is true on only the 0th rank
template<> template<>
void ExceptionMPI_object::test< 6 >() {
  set_test_name( "ErrChkMPI macro throws 0th true only" );

  int numpes;
  MPI_Comm_size( MPI_COMM_WORLD, &numpes );
  if ( numpes > 1 )
    try {
      int peid;
      MPI_Comm_rank( MPI_COMM_WORLD, &peid );
      // cppcheck-suppress duplicateExpression
      ErrChkMPI( peid == 0 ? 1 == 1 : 0 == 1, "msg" )
      fail( "should throw exception" );
    }
    catch ( tk::Exception& ) {
      // exception thrown, test ok
    }
  else
    skip( "in serial, needs multiple PEs" );
}

#if defined(__clang__)
  #pragma clang diagnostic pop
#endif

} // tut::

#endif  // DOXYGEN_GENERATING_OUTPUT
