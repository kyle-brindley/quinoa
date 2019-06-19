// *****************************************************************************
/*!
  \file      src/DiffEq/Dirichlet/MixDirichlet.hpp
  \copyright 2012-2015 J. Bakosi,
             2016-2018 Los Alamos National Security, LLC.,
             2019 Triad National Security, LLC.
             All rights reserved. See the LICENSE file for details.
  \brief     Mixture Dirichlet SDE
  \details   This file implements the time integration of a system of stochastic
    differential equations (SDEs), whose invariant is the [Dirichlet
    distribution](http://en.wikipedia.org/wiki/Dirichlet_distribution), with
    various constraints to model multi-material mixing process in turbulent
    flows.

    In a nutshell, the equation integrated governs a set of scalars,
    \f$0\!\le\!Y_\alpha\f$, \f$\alpha\!=\!1,\dots,N-1\f$,
    \f$\sum_{\alpha=1}^{N-1}Y_\alpha\!\le\!1\f$, as

    @m_class{m-show-m}

    \f[
       \mathrm{d}Y_\alpha(t) = \frac{b_\alpha}{2}\big[S_\alpha Y_N -
       (1-S_\alpha)Y_\alpha\big]\mathrm{d}t + \sqrt{\kappa_\alpha Y_\alpha
       Y_N}\mathrm{d}W_\alpha(t), \qquad \alpha=1,\dots,N-1
    \f]

    @m_class{m-hide-m}

    \f[
       \begin{split}
       \mathrm{d}Y_\alpha(t) = \frac{b_\alpha}{2}\big[S_\alpha Y_N -
       (1-S_\alpha)Y_\alpha\big]\mathrm{d}t + \sqrt{\kappa_\alpha Y_\alpha
       Y_N}\mathrm{d}W_\alpha(t), \\ \alpha=1,\dots,N-1
       \end{split}
    \f]

    with parameter vectors \f$b_\alpha > 0\f$, \f$\kappa_\alpha > 0\f$, and \f$0
    < S_\alpha < 1\f$, and \f$Y_N = 1-\sum_{\alpha=1}^{N-1}Y_\alpha\f$. Here
    \f$\mathrm{d}W_\alpha(t)\f$ is an isotropic vector-valued [Wiener
    process](http://en.wikipedia.org/wiki/Wiener_process) with independent
    increments. The invariant distribution is the Dirichlet distribution,
    provided the parameters of the drift and diffusion terms satisfy
    \f[
      (1-S_1) b_1 / \kappa_1 = \dots = (1-S_{N-1}) b_{N-1} / \kappa_{N-1}.
    \f]
    To keep the invariant distribution Dirichlet, the above constraint on the
    coefficients must be satisfied. For more details on the Dirichlet SDE, see
    https://doi.org/10.1155/2013/842981.
*/
// *****************************************************************************
#ifndef MixDirichlet_h
#define MixDirichlet_h

#include <vector>
#include <cmath>

#include "InitPolicy.hpp"
#include "MixDirichletCoeffPolicy.hpp"
#include "RNG.hpp"
#include "Particles.hpp"

namespace walker {

extern ctr::InputDeck g_inputdeck;
extern std::map< tk::ctr::RawRNGType, tk::RNG > g_rng;

//! Number of derived variables computed by the MixDirichlet SDE
const std::size_t MIXDIR_NUMDERIVED = 2;

//! \brief MixDirichlet SDE used polymorphically with DiffEq
//! \details The template arguments specify policies and are used to configure
//!   the behavior of the class. The policies are:
//!   - Init - initialization policy, see DiffEq/InitPolicy.h
//!   - Coefficients - coefficients policy, see DiffEq/DirCoeffPolicy.h
template< class Init, class Coefficients >
class MixDirichlet {

  private:
    using ncomp_t = tk::ctr::ncomp_type;
    using eq = tag::mixdirichlet;

  public:
    //! \brief Constructor
    //! \param[in] c Index specifying which MixDirichlet SDE to construct. There
    //!   can be multiple dirichlet ... end blocks in a control file. This index
    //!   specifies which MixDirichlet SDE to instantiate. The index corresponds
    //!   to the order in which the dirichlet ... end blocks are given the
    //!   control file.
    explicit MixDirichlet( ncomp_t c ) :
      m_c( c ),
      m_depvar( g_inputdeck.get< tag::param, eq, tag::depvar >().at(c) ),
      // subtract the number of derived variables computed, see advance()
      m_ncomp( g_inputdeck.get< tag::component >().get< eq >().at(c) -
               MIXDIR_NUMDERIVED ),
      m_offset(
        g_inputdeck.get< tag::component >().offset< eq >(c) ),
      m_rng( g_rng.at( tk::ctr::raw(
        g_inputdeck.get< tag::param, eq, tag::rng >().at(c) ) ) ),
      m_norm( g_inputdeck.get< tag::param, eq, tag::normalization >().at(c) ),
      m_b(),
      m_S(),
      m_kprime(),
      m_k(),
      m_rho(),
      m_r(),
      coeff( m_ncomp,
             m_norm,
             g_inputdeck.get< tag::param, eq, tag::b >().at(c),
             g_inputdeck.get< tag::param, eq, tag::S >().at(c),
             g_inputdeck.get< tag::param, eq, tag::kappaprime >().at(c),
             g_inputdeck.get< tag::param, eq, tag::rho >().at(c),
             m_b, m_S, m_kprime, m_rho, m_r, m_k ) {}

    //! Initalize SDE, prepare for time integration
    //! \param[in] stream Thread (or more precisely stream) ID 
    //! \param[in,out] particles Array of particle properties 
    void initialize( int stream, tk::Particles& particles ) {

      // Ensure the size of the parameter vector holding the pure-fluid
      // densities is N = K+1 = m_ncomp+1
      Assert( m_rho.size() == m_ncomp+1, "Size mismatch" );

      //! Set initial conditions using initialization policy
      Init::template init< eq >( g_inputdeck, m_rng, stream, particles, m_c,
                                 m_ncomp, m_offset );

      // Initialize derived instantaneous variables
      const auto npar = particles.nunk();
      for (auto p=decltype(npar){0}; p<npar; ++p) {
        auto yn = Yn( particles, p );
        // violating boundedness here is a hard error as indicates a problem
        // with the initial conditions
        if (yn < 0.0 || yn > 1.0) Throw( "Nth scalar of IC out of bounds" );
        derived( particles, p, yn );
      }
    }

    //! \brief Advance particles according to the MixDirichlet SDE
    //! \param[in,out] particles Array of particle properties
    //! \param[in] stream Thread (or more precisely stream) ID
    //! \param[in] dt Time step size
    //! \param[in] moments Map of statistical moments
    void advance( tk::Particles& particles,
                  int stream,
                  tk::real dt,
                  tk::real,
                  const std::map< tk::ctr::Product, tk::real >& moments )
    {
      // Update SDE coefficients
      coeff.update( m_depvar, m_ncomp, moments, m_rho, m_r, m_kprime, m_b, m_k,
                    m_S );

      // Advance particles
      const auto npar = particles.nunk();
      for (auto p=decltype(npar){0}; p<npar; ++p) {
        // Generate Gaussian random numbers with zero mean and unit variance
        std::vector< tk::real > dW( m_ncomp );
        m_rng.gaussian( stream, m_ncomp, dW.data() );

        // compute Nth scalar
        auto yn = Yn( particles, p );
        // violating boundedness here is a hard error as indicates a problem
        // with acception-rejection logic within this time-advancement loop
        if (yn < 0.0 || yn > 1.0) Throw( "Nth scalar out of bounds" );

        // Advance first m_ncomp (K=N-1) scalars
        bool accept = true;
        std::vector< tk::real > Y( m_ncomp, 0.0 );
        for (ncomp_t i=0; i<m_ncomp; ++i) {
          tk::real y = particles( p, i, m_offset );
          tk::real d = m_k[i] * y * yn * dt;
          if (d < 0.0) Throw( "Negative diffusion term" );
          d = std::sqrt( d );
          y += 0.5*m_b[i]*( m_S[i]*yn - (1.0-m_S[i])*y )*dt + d*dW[i];
          // if even a single y is out of bounds, will re-advance this particle
          if (y < 0.0 || y > 1.0) {
            accept = false;
            i = m_ncomp;
          } else {
            Y[i] = y;   // save first N-1 advanced scalars
          }
        }

        // if none of the N-1 scalars are out of bounds, recompute Nth scalar
        if (accept) {
          // re-compute Nth scalar (from the just-advanced N-1 scalars)
          yn = Yn( Y );
          // if the Nth scalar is out of bounds, will re-advance this particle
          if (yn < 0.0 || yn > 1.0) {
            accept = false;
          } else {
            // all N scalars are within bounds, accept this advance
            for (ncomp_t i=0; i<m_ncomp; ++i)
              particles( p, i, m_offset ) = Y[i];
            // compute derived particle values
            derived( particles, p, yn );
          }
        }

        // re-advance this particle if a single scalar was out of bounds
        if (!accept) --p;
      }
    }

  private:
    const ncomp_t m_c;                  //!< Equation system index
    const char m_depvar;                //!< Dependent variable
    const ncomp_t m_ncomp;              //!< Number of components
    const ncomp_t m_offset;             //!< Offset SDE operates from
    const tk::RNG& m_rng;               //!< Random number generator
    const ctr::NormalizationType m_norm;//!< Normalization type

    //! Coefficients
    std::vector< kw::sde_b::info::expect::type > m_b;
    std::vector< kw::sde_S::info::expect::type > m_S;
    std::vector< kw::sde_kappa::info::expect::type > m_kprime;
    std::vector< kw::sde_kappa::info::expect::type > m_k;
    std::vector< kw::sde_rho::info::expect::type > m_rho;
    std::vector< kw::sde_r::info::expect::type > m_r;

    //! Coefficients policy
    Coefficients coeff;

    //! Compute Nth mass fraction
    //! \param[in] Y N-1 scalars as a vector
    //! \return The Nth scalar
    tk::real Yn( const std::vector< tk::real >& Y ) const {
      Assert( Y.size() == m_ncomp, "Size mismatch" );
      tk::real yn = 1.0;
      for (ncomp_t i=0; i<m_ncomp; ++i) yn -= Y[i];
      return yn;
    }

    //! Compute Nth mass fraction
    //! \param[in] particles Array of particle properties
    //! \param[in] p Particle index
    //! \return The Nth scalar
    tk::real Yn( const tk::Particles& particles, ncomp_t p ) const {
      tk::real yn = 1.0;
      for (ncomp_t i=0; i<m_ncomp; ++i) yn -= particles( p, i, m_offset );
      return yn;
    }

    //! \brief Return density for mass fractions
    //! \details This function returns the instantaneous density, rho,
    //!   based on the multiple mass fractions, Y_c.
    //! \param[in] particles Array of particle properties
    //! \param[in] p Particle index
    //! \param[in] Yn Nth particle mass fraction
    //! \return Instantaneous value of the density, rho
    //! \details This is computed based 1/rho = sum_{i=1}^N Y_i/R_i, where R_i
    //!   are the constant pure-fluid densities and the Y_i are the mass
    //!   fractions, of the N materials. (Note that we only solve for K=N-1
    //!   mass fractions.)
    tk::real rho( const tk::Particles& particles, ncomp_t p, tk::real Yn ) const
    {
      // start computing density
      tk::real d = 0.0;
      for (ncomp_t i=0; i<m_ncomp; ++i) {
        d += particles( p, i, m_offset ) / m_rho[i];
      }
      // finish computing density
      d += Yn / m_rho[m_ncomp];
      // return particle density
      return 1.0 / d;
    }

    //! \brief Return specific volume for mass fractions
    //! \details This function returns the instantaneous specific volume, v,
    //!   based on the multiple mass fractions, Y_c.
    //! \param[in] particles Array of particle properties
    //! \param[in] p Particle index
    //! \param[in] Yn Nth particle mass fraction
    //! \return Instantaneous value of the specific volume, v
    //! \details This is computed based v = 1/rho = sum_{i=1}^N Y_i/R_i,
    //!   where R_i are the constant pure-fluid densities and the Y_i are the
    //!   mass fractions, of the N materials. (Note that we only solve for K=N-1
    //!   mass fractions.)
    tk::real vol( const tk::Particles& particles, ncomp_t p, tk::real Yn ) const
    {
      return 1.0 / rho( particles, p );
    }

    //! Compute instantaneous values derived from particle mass fractions
    //! \param[in,out] particles Particle properties array
    //! \param[in] p Particle index
    //! \param[in] Yn Nth particle mass fraction
    void derived( tk::Particles& particles, ncomp_t p, tk::real Yn ) const {
      // compute instantaneous fluid-density based on particle mass fractions
      auto density = rho( particles, p, Yn );
      // Compute and store instantaneous density
      particles( p, m_ncomp, m_offset ) = density;
      // Store instantaneous specific volume
      particles( p, m_ncomp+1, m_offset ) = 1.0/density;
    }

};

} // walker::

#endif // MixDirichlet_h
