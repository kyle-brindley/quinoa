//******************************************************************************
/*!
  \file      src/Main/Driver.h
  \author    J. Bakosi
  \date      Tue Jan 14 08:53:01 2014
  \copyright Copyright 2005-2012, Jozsef Bakosi, All rights reserved.
  \brief     Driver base
  \details   Driver base
*/
//******************************************************************************
#ifndef Driver_h
#define Driver_h

#include <list>
#include <map>

#include <Quinoa/Options/RNG.h>
#include <RNG.h>
#include <RNGSSE.h>

namespace tk {

//! Random number generator factory type
using RNGFactory = std::map< quinoa::ctr::RNGType, std::function<tk::RNG*()> >;

//! Driver base class
class Driver {

  public:
    //! Constructor
    explicit Driver() = default;

    //! Destructor
    virtual ~Driver() noexcept = default;

    //! Execute
    virtual void execute() const = 0;

    //! Register random number generators into factory
    void initRNGFactory( tk::RNGFactory& factory,
                         const quinoa::ctr::RNG& opt,
                         std::list< quinoa::ctr::RNGType >& reg,
                         int nthreads,
                         const quinoa::ctr::MKLRNGParameters& mklparam,
                         const quinoa::ctr::RNGSSEParameters& rngsseparam );

  private:
    //! Don't permit copy constructor
    Driver(const Driver&) = delete;
    //! Don't permit assigment constructor
    Driver& operator=(const Driver&) = delete;
    //! Don't permit move constructor
    Driver(Driver&&) = delete;
    //! Don't permit move assignment
    Driver& operator=(Driver&&) = delete;

   #ifdef HAS_MKL
   //! Register MKL RNGs into factory
   void regMKL( tk::RNGFactory& factory,
                const quinoa::ctr::RNG& opt,
                std::list< quinoa::ctr::RNGType >& reg,
                int nthreads,
                const quinoa::ctr::MKLRNGParameters& mklparam );
   #endif

   //! Register RNGSSE RNGs into factory
   void regRNGSSE( tk::RNGFactory& factory,
                   const quinoa::ctr::RNG& opt,
                   std::list< quinoa::ctr::RNGType >& reg,
                   int nthreads,
                   const quinoa::ctr::RNGSSEParameters& param );
};

} // namespace tk

#endif // Driver_h
