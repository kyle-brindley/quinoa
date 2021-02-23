// *****************************************************************************
/*!
  \file      tests/unit/Control/Options/TestMKLUniformMethod.cpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019-2021 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Unit tests for Control/Options/MKLUniformMethod
  \details   Unit tests for Control/Options/MKLUniformMethod
*/
// *****************************************************************************

#include "NoWarning/tut.hpp"

#include "TUTConfig.hpp"
#include "Options/MKLUniformMethod.hpp"

#ifndef DOXYGEN_GENERATING_OUTPUT

namespace tut {

//! All tests in group inherited from this base
struct MKLUniformMethod_common {
  MKLUniformMethod_common() : m() {}
  const tk::ctr::MKLUniformMethod m;
};

//! Test group shortcuts
using MKLUniformMethod_group =
  test_group< MKLUniformMethod_common, MAX_TESTS_IN_GROUP >;
using MKLUniformMethod_object = MKLUniformMethod_group::object;

//! Define test group
static
MKLUniformMethod_group MKLUniformMethod( "Control/Options/MKLUniformMethod" );

//! Test definitions for group

//! Test that member function param() finds MKL parameter for method type
template<> template<>
void MKLUniformMethod_object::test< 1 >() {
  set_test_name( "param() finds MKL param" );
  ensure( "cannot find parameter",
          m.param( tk::ctr::MKLUniformMethodType::STANDARD ) ==
            VSL_RNG_METHOD_UNIFORM_STD );
}

//! Test that member function param() throws in DEBUG mode if can't find param
template<> template<>
void MKLUniformMethod_object::test< 2 >() {
  set_test_name( "param() throws" );

  try {

    m.param( static_cast< tk::ctr::MKLUniformMethodType >( 234 ) );
    #ifndef NDEBUG
    fail( "should throw exception in DEBUG mode" );
    #endif

  } catch ( tk::Exception& e ) {
    // exception thrown in DEBUG mode, test ok, Assert skipped in RELEASE mode
    // if any other type of exception is thrown, test fails with except
    ensure( std::string("wrong exception thrown: ") + e.what(),
            std::string( e.what() ).find( "Cannot find parameter" ) !=
              std::string::npos );
  }
}

//! Test copy constructor
template<> template<>
void MKLUniformMethod_object::test< 3 >() {
  set_test_name( "copy constructor" );

  tk::ctr::MKLUniformMethod c( m );
  std::vector< tk::ctr::MKLUniformMethod > v;
  v.push_back( c );
  ensure( "copy constructor used to push_back a MKLUniformMethod object to a "
          "std::vector",
          v[0].param( tk::ctr::MKLUniformMethodType::ACCURATE ) ==
            (VSL_RNG_METHOD_UNIFORM_STD_ACCURATE) );
}

//! Test move constructor
template<> template<>
void MKLUniformMethod_object::test< 4 >() {
  set_test_name( "move constructor" );

  tk::ctr::MKLUniformMethod c( m );
  std::vector< tk::ctr::MKLUniformMethod > v;
  v.emplace_back( std::move(c) );
  ensure( "move constructor used to emplace_back a MKLUniformMethod object to "
          "a std::vector",
           v[0].param( tk::ctr::MKLUniformMethodType::ACCURATE ) ==
             (VSL_RNG_METHOD_UNIFORM_STD_ACCURATE) );
}

//! Test copy assignment
template<> template<>
void MKLUniformMethod_object::test< 5 >() {
  set_test_name( "copy assignment" );

  tk::ctr::MKLUniformMethod c;
  c = m;
  ensure( "find param of copy-assigned MKLUniformMethod",
          c.param( tk::ctr::MKLUniformMethodType::ACCURATE ) ==
             (VSL_RNG_METHOD_UNIFORM_STD_ACCURATE) );
}

//! Test move assignment
template<> template<>
void MKLUniformMethod_object::test< 6 >() {
  set_test_name( "move assignment" );

  tk::ctr::MKLUniformMethod c;
  c = std::move( m );
  ensure( "find param of move-assigned MKLUniformMethod",
          c.param( tk::ctr::MKLUniformMethodType::ACCURATE ) ==
             (VSL_RNG_METHOD_UNIFORM_STD_ACCURATE) );
}

} // tut::

#endif  // DOXYGEN_GENERATING_OUTPUT
