// *****************************************************************************
/*!
  \file      tests/unit/Base/TestTaggedTuple.cpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019-2021 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Unit tests for tk::TaggedTuple
  \details   Unit tests for tk::TaggedTuple
*/
// *****************************************************************************

#include "NoWarning/tut.hpp"

#include "TUTConfig.hpp"
#include "TaggedTuple.hpp"
#include "Types.hpp"

#ifndef DOXYGEN_GENERATING_OUTPUT

namespace tut {

//! All tests in group inherited from this base
struct TaggedTuple_common {
  // Tags
  struct name {};
  struct age {};
  struct email {};
  struct tag1 {};
  struct tag2 {};
  struct tag3 {};
  struct tag4 {};
  struct tag5 {};
  struct tag6 {};
  struct tag7 {};
  struct tag8 {};
  struct tag9 {};

  // Define a tagged tuple: odd template arguments are tags, even ones are types
  using record = tk::TaggedTuple< brigand::list< name,  std::string,
                                                 age,   int,
                                                 email, std::string > >;

  // Declare a record, initialize with an std::initializer_list
  record tup{{ "Bob", 32, "bob@bob.bob" }};

  using map_value_tuple = tk::TaggedTuple< brigand::list< tag1, tk::real > >;
  using tuple1 = tk::TaggedTuple< brigand::list<
                   tag1, std::string,
                   tag2, int,
                   tag5, std::vector< int >,
                   tag6, std::vector< std::vector< int > >,
                   tag9, std::vector< std::vector< std::vector< int > > >,
                   tag7, std::map< int, std::string >,
                   tag8, std::map< int, map_value_tuple > > >;
  using tuple2 = tk::TaggedTuple< brigand::list<
                   tag1, std::string,
                   tag2, int,
                   tag3, tuple1,
                   tag5, std::vector< int >,
                   tag6, std::vector< std::vector< int > >,
                   tag9, std::vector< std::vector< std::vector< int > > >,
                   tag7, std::map< int, std::string >,
                   tag8, std::map< int, map_value_tuple > > >;
  using control = tk::TaggedTuple< brigand::list<
                   tag1, std::string,
                   tag2, int,
                   tag3, tuple1,
                   tag4, tuple2,
                   tag5, std::vector< int >,
                   tag6, std::vector< std::vector< int > >,
                   tag9, std::vector< std::vector< std::vector< int > > >,
                   tag7, std::map< int, std::string >,
                   tag8, std::map< int, map_value_tuple > > >;

  double precision = std::numeric_limits< tk::real >::epsilon();

  //! Lambda to compare two maps of TaggedTuple objects
  void compare( const std::map< int, map_value_tuple >& lhs,
                const std::map< int, map_value_tuple >& rhs )
  {
    ensure_equals( "sizes of lhs and rhs not equal", lhs.size(), rhs.size() );
    for (const auto& e : lhs) {
      auto r = rhs.find( e.first );
      ensure( "key in lhs not found in rhs", r != end(rhs) );
      auto v = r->second.template get< tag1 >();
      ensure_equals( "value in lhs not equal value in rhs",
                     e.second.template get< tag1 >(), v, precision );
    }
  }
};

//! Test group shortcuts
using TaggedTuple_group = test_group< TaggedTuple_common, MAX_TESTS_IN_GROUP >;
using TaggedTuple_object = TaggedTuple_group::object;

//! Define test group
static TaggedTuple_group TaggedTuple( "Base/TaggedTuple" );

//! Test definitions for group

//! Test const-ref accessor of TaggedTuple
template<> template<>
void TaggedTuple_object::test< 1 >() {
  set_test_name( "const-ref accessors" );

  // Define a struct holding a record with const-ref accessors
  struct A {
    explicit A( const record& r ) : m_rec( r ) {}
    const std::string& getName() const { return m_rec.get< name >(); }
    const int& getAge() const { return m_rec.get< age >(); }
    const std::string& getEmail() const { return m_rec.get< email >(); }
    const record m_rec;
  };

  // Use const-ref accessors to read out the initialized data
  A a( tup );
  ensure_equals( "const-ref accessor to std::string", a.getName(), "Bob" );
  ensure_equals( "const-ref accessor to int", a.getAge(), 32 );
  ensure_equals( "const-ref accessor to std::string",
                 a.getEmail(), "bob@bob.bob" );
}

//! Test non-const-ref accessors of TaggedTuple
template<> template<>
void TaggedTuple_object::test< 2 >() {
  set_test_name( "non-const-ref accessors" );

  // Use (non-const) ref accessors to read out the initialized data
  ensure_equals( "non-const-ref accessor to std::string",
                 tup.get< name >(), "Bob" );
  ensure_equals( "non-const-ref accessor to int",
                 tup.get< age >(), 32 );
  ensure_equals( "non-const-ref accessor to std::string",
                 tup.get< email >(), "bob@bob.bob" );
}

//! Test set(const char*) with const rvalue argument of TaggedTuple
template<> template<>
void TaggedTuple_object::test< 3 >() {
  set_test_name( "get(const char*) - const rvalue arg" );

  record t{{ "Bob", 32, "bob@bob.bob" }};
  t.get< name >() = "Alice";     // const rvalue
  ensure_equals( "get() after set(const char*)", t.get< name >(), "Alice" );
}

//! Test set(std::string) with non-const lvalue argument of TaggedTuple
template<> template<>
void TaggedTuple_object::test< 4 >() {
  set_test_name( "get(std::string) - non-const lvalue arg" );

  record t{{ "Bob", 32, "bob@bob.bob" }};
  std::string n( "Alice" );
  t.get< name >() = n;           // non-const lvalue
  ensure_equals( "get() after set(std::string)", t.get< name >(), "Alice" );
  ensure_equals( "original source kept intact", n, "Alice" );
}

//! Test set(std::string) with const lvalue argument of TaggedTuple
template<> template<>
void TaggedTuple_object::test< 5 >() {
  set_test_name( "get(const std::string) - const lvalue" );

  record t{{ "Bob", 32, "bob@bob.bob" }};
  const std::string n( "Alice" );
  t.get< name >() = n;           // const lvalue
  ensure_equals( "get() after set(std::string)", t.get< name >(), "Alice" );
  ensure_equals( "original source kept intact", n, "Alice" );
}

//! Test set(std::string) with rvalue argument of TaggedTuple
template<> template<>
void TaggedTuple_object::test< 6 >() {
  set_test_name( "get(std::string&&) - rvalue ref arg" );

  record t{{ "Bob", 32, "bob@bob.bob" }};
  std::string n( "Alice" );
  t.get< name >() = std::move(n);           // rvalue reference
  ensure_equals( "get() after set(std::string)", t.get< name >(), "Alice" );
  // n here should still be in a valid but unspecified state, so we try to use
  // it, if that's not the case, the constructor or the copy assignment throws.
  // Also suppress cppcheck warning on accessing a moved-from variable, because
  // here we are testing if the moved-from variable, n, is still in a valid
  // state after the move and wether if equating n with k really zeroes k out.
  std::string k( "abc" );
  // cppcheck-suppress accessMoved
  k = n;
  ensure_equals( "original source in valid but unspecified state", k, n );
}

//! Test size of TaggedTuple
template<> template<>
void TaggedTuple_object::test< 7 >() {
  set_test_name( "TaggedTuple::size()" );
  static_assert( record::size() == 3, "Size of tuple incorrect" );
  ensure_equals( "TaggedTuple::size() incorrect", record().size(), 3UL );
}

//! Test set(), const-ref get() at three levels
template<> template<>
void TaggedTuple_object::test< 8 >() {
  set_test_name( "set(), const-ref get() at three depths" );

  control c;
  c.get< tag1 >() = "blah1";
  c.get< tag3, tag1 >() ="blah2";
  c.get< tag4, tag3, tag1 >() = "blah3";

  ensure_equals( "get() 1st level", c.get< tag1 >(), "blah1" );
  ensure_equals( "get() 2nd level", c.get< tag3, tag1 >(), "blah2" );
  ensure_equals( "get() 3rd level", c.get< tag4, tag3, tag1 >(), "blah3" );
}

//! Test set(), rvalue get() at three levels
template<> template<>
void TaggedTuple_object::test< 9 >() {
  set_test_name( "set(), rvalue get() at three depths" );

  control c;
  c.get< tag1 >() = "blah1";
  c.get< tag3, tag1 >() = "blah2";
  c.get< tag4, tag3, tag1 >() = "blah3";

  ensure_equals( "get() 1st level", c.get< tag1 >(), "blah1" );
  ensure_equals( "get() 2nd level", c.get< tag3, tag1 >(), "blah2" );
  ensure_equals( "get() 3rd level", c.get< tag4, tag3, tag1 >(), "blah3" );
}

//! Test store() at three levels
template<> template<>
void TaggedTuple_object::test< 10 >() {
  set_test_name( "store() at three depths" );

  control c;
  c.store< tag2 >( "1" );
  c.store< tag3, tag2 >( "2" );
  c.store< tag4, tag3, tag2 >( "3" );

  ensure_equals( "get() 1st level", c.get< tag2 >(), 1 );
  ensure_equals( "get() 2nd level", c.get< tag3, tag2 >(), 2 );
  ensure_equals( "get() 3rd level", c.get< tag4, tag3, tag2 >(), 3 );
}

//! Test push_back() at three levels
template<> template<>
void TaggedTuple_object::test< 11 >() {
  set_test_name( "push_back() at three depths" );

  control c;
  c.get< tag5 >().push_back( 1 );
  c.get< tag5 >().push_back( 1 );
  c.get< tag3, tag5 >().push_back( 2 );
  c.get< tag3, tag5 >().push_back( 2 );
  c.get< tag4, tag3, tag5 >().push_back( 3 );
  c.get< tag4, tag3, tag5 >().push_back( 3 );

  ensure_equals( "vector size at 1st level", c.get< tag5 >().size(), 2 );
  ensure_equals( "vector size at 2nd level", c.get< tag3, tag5 >().size(), 2 );
  ensure_equals( "vector size at 3rd level",
                 c.get< tag4, tag3, tag5 >().size(), 2 );

  ensure( "vector elements incorrect at 1st level",
          c.get< tag5 >() == std::vector< int >{ 1, 1 } );
  ensure( "vector elements incorrect at 2nd level",
          c.get< tag3, tag5 >() == std::vector< int >{ 2, 2 } );
  ensure( "vector elements incorrect at 3rd level",
          c.get< tag4, tag3, tag5 >() == std::vector< int >{ 3, 3 } );
}

//! Test push_back_back() at three levels
template<> template<>
void TaggedTuple_object::test< 12 >() {
  set_test_name( "push_back_back() at three depths" );

  control c;
  c.get< tag6 >().push_back( {} );  // create an outer vector element
  c.get< tag6 >().back().push_back( 1 );     // push to inner vector
  c.get< tag6 >().back().push_back( 1 );
  c.get< tag3, tag6 >().push_back( {} );
  c.get< tag3, tag6 >().back().push_back( 2 );
  c.get< tag3, tag6 >().back().push_back( 2 );
  c.get< tag4, tag3, tag6 >().push_back( {} );
  c.get< tag4, tag3, tag6 >().back().push_back( 3 );
  c.get< tag4, tag3, tag6 >().back().push_back( 3 );

  ensure_equals( "vector size at 1st level",
                 c.get< tag6 >().back().size(), 2 );
  ensure_equals( "vector size at 2nd level",
                 c.get< tag3, tag6 >().back().size(), 2 );
  ensure_equals( "vector size at 3rd level",
                 c.get< tag4, tag3, tag6 >().back().size(), 2 );

  ensure( "vector elements incorrect at 1st level",
          c.get< tag6 >().back() == std::vector< int >{ 1, 1 } );
  ensure( "vector elements incorrect at 2nd level",
          c.get< tag3, tag6 >().back() == std::vector< int >{ 2, 2 } );
  ensure( "vector elements incorrect at 3rd level",
          c.get< tag4, tag3, tag6 >().back() == std::vector< int >{ 3, 3 } );
}

//! Test store_back() at three levels
template<> template<>
void TaggedTuple_object::test< 13 >() {
  set_test_name( "store_back() at three depths" );

  control c;
  c.store_back< tag5 >( "1" );
  c.store_back< tag5 >( "1" );
  c.store_back< tag3, tag5 >( "2" );
  c.store_back< tag3, tag5 >( "2" );
  c.store_back< tag4, tag3, tag5 >( "3" );
  c.store_back< tag4, tag3, tag5 >( "3" );

  ensure_equals( "vector size at 1st level", c.get< tag5 >().size(), 2 );
  ensure_equals( "vector size at 2nd level", c.get< tag3, tag5 >().size(), 2 );
  ensure_equals( "vector size at 3rd level",
                 c.get< tag4, tag3, tag5 >().size(), 2 );

  ensure( "vector elements incorrect at 1st level",
          c.get< tag5 >() == std::vector< int >{ 1, 1 } );
  ensure( "vector elements incorrect at 2nd level",
          c.get< tag3, tag5 >() == std::vector< int >{ 2, 2 } );
  ensure( "vector elements incorrect at 3rd level",
          c.get< tag4, tag3, tag5 >() == std::vector< int >{ 3, 3 } );
}

//! Test store_back_back() at three levels
template<> template<>
void TaggedTuple_object::test< 14 >() {
  set_test_name( "store_back_back() at three depths" );

  control c;
  c.get< tag6 >().push_back( {} );  // create an outer vector element
  c.store_back_back< tag6 >( "1" );     // push to inner vector
  c.store_back_back< tag6 >( "1" );
  c.get< tag3, tag6 >().push_back( {} );
  c.store_back_back< tag3, tag6 >( "2" );
  c.store_back_back< tag3, tag6 >( "2" );
  c.get< tag4, tag3, tag6 >().push_back( {} );
  c.store_back_back< tag4, tag3, tag6 >( "3" );
  c.store_back_back< tag4, tag3, tag6 >( "3" );

  ensure_equals( "vector size at 1st level",
                 c.get< tag6 >().back().size(), 2 );
  ensure_equals( "vector size at 2nd level",
                 c.get< tag3, tag6 >().back().size(), 2 );
  ensure_equals( "vector size at 3rd level",
                 c.get< tag4, tag3, tag6 >().back().size(), 2 );

  ensure( "vector elements incorrect at 1st level",
          c.get< tag6 >().back() == std::vector< int >{ 1, 1 } );
  ensure( "vector elements incorrect at 2nd level",
          c.get< tag3, tag6 >().back() == std::vector< int >{ 2, 2 } );
  ensure( "vector elements incorrect at 3rd level",
          c.get< tag4, tag3, tag6 >().back() == std::vector< int >{ 3, 3 } );
}

//! Test store_back_back_back() at three levels
template<> template<>
void TaggedTuple_object::test< 15 >() {
  set_test_name( "store_back_back_back() at three depths" );

  control c;
  c.get< tag9 >().push_back( {} );
  c.get< tag9 >().back().push_back( {} );
  c.store_back_back_back< tag9 >( "1" );
  c.store_back_back_back< tag9 >( "1" );
  c.get< tag3, tag9 >().push_back( {} );
  c.get< tag3, tag9 >().back().push_back( {} );
  c.store_back_back_back< tag3, tag9 >( "2" );
  c.store_back_back_back< tag3, tag9 >( "2" );
  c.get< tag4, tag3, tag9 >().push_back( {} );
  c.get< tag4, tag3, tag9 >().back().push_back( {} );
  c.store_back_back_back< tag4, tag3, tag9 >( "3" );
  c.store_back_back_back< tag4, tag3, tag9 >( "3" );

  ensure_equals( "vector size at 1st level",
                 c.get< tag9 >().back().back().size(), 2 );
  ensure_equals( "vector size at 2nd level",
                 c.get< tag3, tag9 >().back().back().size(), 2 );
  ensure_equals( "vector size at 3rd level",
                 c.get< tag4, tag3, tag9 >().back().back().size(), 2 );

  ensure( "vector elements incorrect at 1st level",
          c.get< tag9 >().back().back() == std::vector<int>{1,1} );
  ensure( "vector elements incorrect at 2nd level",
          c.get< tag3, tag9 >().back().back() == std::vector<int>{2,2} );
  ensure( "vector elements incorrect at 3rd level",
          c.get< tag4, tag3, tag9 >().back().back() == std::vector<int>{3,3} );
}

//! Test insert() at three levels
template<> template<>
void TaggedTuple_object::test< 16 >() {
  set_test_name( "insert() at three depths" );

  control c;
  c.insert< tag7 >( 1, "one" );
  c.insert< tag7 >( 2, "two" );
  c.insert< tag7 >( 10, "" );
  c.insert< tag3, tag7 >( 3, "three" );
  c.insert< tag3, tag7 >( 4, "four" );
  c.insert< tag3, tag7 >( 10, "" );
  c.insert< tag4, tag3, tag7 >( 5, "five" );
  c.insert< tag4, tag3, tag7 >( 6, "six" );
  c.insert< tag4, tag3, tag7 >( 10, "" );

  ensure_equals( "map size at 1st level", c.get< tag7 >().size(), 3 );
  ensure_equals( "map size at 2nd level", c.get< tag3, tag7 >().size(), 3 );
  ensure_equals( "map size at 3rd level",
                 c.get< tag4, tag3, tag7 >().size(), 3 );

  ensure( "map elements incorrect at 1st level",
          c.get< tag7 >() ==
            std::map< int, std::string >{ {1,"one"}, {2,"two"}, {10,""} } );
  ensure( "map elements incorrect at 2nd level",
          c.get< tag3, tag7 >() ==
            std::map< int, std::string >{ {3,"three"}, {4,"four"}, {10,""} } );
  ensure( "map elements incorrect at 3rd level",
          c.get< tag4, tag3, tag7 >() ==
            std::map< int, std::string >{ {5,"five"}, {6,"six"}, {10,""} } );
}

//! Test insert_field() at three levels, with conversion from string
template<> template<>
void TaggedTuple_object::test< 17 >() {
  set_test_name( "insert_field() at three depths" );

  control c;
  c.insert_field< tag1, tk::real, tag8 >( 1, "-1.2" );
  c.insert_field< tag1, tk::real, tag8 >( 2, "3.14" );
  c.insert_field< tag1, tk::real, tag3, tag8 >( 3, "2.1" );
  c.insert_field< tag1, tk::real, tag3, tag8 >( 4, "3e-3" );
  c.insert_field< tag1, tk::real, tag4, tag3, tag8 >( 5, "-2.3" );
  c.insert_field< tag1, tk::real, tag4, tag3, tag8 >( 6, "-10" );

  ensure_equals( "map size at 1st level", c.get< tag8 >().size(), 2 );
  ensure_equals( "map size at 2nd level", c.get< tag3, tag8 >().size(), 2 );
  ensure_equals( "map size at 3rd level",
                 c.get< tag4, tag3, tag8 >().size(), 2 );

  compare( c.get< tag8 >(), 
           std::map< int, map_value_tuple >{ {1,map_value_tuple{-1.2}},
                                             {2,map_value_tuple{3.14}} } );
  compare( c.get< tag3, tag8 >(), 
           std::map< int, map_value_tuple >{ {3,map_value_tuple{2.1}},
                                             {4,map_value_tuple{3e-3}} } );
  compare( c.get< tag4, tag3, tag8 >(), 
           std::map< int, map_value_tuple >{ {5,map_value_tuple{-2.3}},
                                             {6,map_value_tuple{-10}} } );
}

//! Test insert_field() at three levels, without conversion
template<> template<>
void TaggedTuple_object::test< 18 >() {
  set_test_name( "insert_field() at three depths, with conversion" );

  control c;
  c.insert_field< tag1, tk::real, tag8 >( 1, 1.2 );
  c.insert_field< tag1, tk::real, tag8 >( 2, 3.14 );
  c.insert_field< tag1, tk::real, tag3, tag8 >( 3, 2.1 );
  c.insert_field< tag1, tk::real, tag3, tag8 >( 4, 3e-3 );
  c.insert_field< tag1, tk::real, tag4, tag3, tag8 >( 5, -2.3 );
  c.insert_field< tag1, tk::real, tag4, tag3, tag8 >( 6, -10 );

  ensure_equals( "map size at 1st level", c.get< tag8 >().size(), 2 );
  ensure_equals( "map size at 2nd level", c.get< tag3, tag8 >().size(), 2 );
  ensure_equals( "map size at 3rd level",
                 c.get< tag4, tag3, tag8 >().size(), 2 );

  compare( c.get< tag8 >(), 
           std::map< int, map_value_tuple >{ {1,map_value_tuple{1.2}},
                                             {2,map_value_tuple{3.14}} } );
  compare( c.get< tag3, tag8 >(),
           std::map< int, map_value_tuple >{ {3,map_value_tuple{2.1}},
                                             {4,map_value_tuple{3e-3}} } );
  compare( c.get< tag4, tag3, tag8 >(),
           std::map< int, map_value_tuple >{ {5,map_value_tuple{-2.3}},
                                             {6,map_value_tuple{-10}} } );
}

//! Test T convert( str ) feeding garbage
template<> template<>
void TaggedTuple_object::test< 19 >() {
  set_test_name( "T convert(str) feeding garbage" );

  try {
    control().insert_field< tag1, tk::real, tag8 >( 1, "a" );
    fail( "should throw exception" );
  }
  catch ( tk::Exception& ) {
    // exception thrown, test ok
  }
}

//! Test T convert( str ) feeding convertible value
template<> template<>
void TaggedTuple_object::test< 20 >() {
  set_test_name( "T convert(str)" );

  try {
    control c;
    c.insert_field< tag1, tk::real, tag8 >( 1, "345" );
    const auto& m = c.get< tag8 >();
    auto i = m.find( 1 );
    Assert( i != end(m), "Key not found in map in TaggedTuple" );
    ensure_equals( "conversion", i->second.get< tag1 >(), 345.0, precision );
  }
  catch ( tk::Exception& ) {
    fail( "should not throw exception" );
  }
}

} // tut::

#endif  // DOXYGEN_GENERATING_OUTPUT
