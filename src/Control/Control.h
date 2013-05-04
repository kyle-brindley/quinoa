//******************************************************************************
/*!
  \file      src/Control/Control.h
  \author    J. Bakosi
  \date      Wed 01 May 2013 08:54:56 PM MDT
  \copyright Copyright 2005-2012, Jozsef Bakosi, All rights reserved.
  \brief     Main control category
  \details   Main control catgeory
*/
//******************************************************************************
#ifndef Control_h
#define Control_h

#include <string>

#include <QuinoaTypes.h>
#include <ControlTypes.h>
#include <BackAssociate.h>
#include <Defaults.h>

namespace Quinoa {

//! Control base
class Control {

  private:
    control::Bundle m_data;        //! Data parsed
    control::BoolBundle m_booldata;//! Flags indicating if data was parsed

  public:
    //! Constructor
    explicit Control() : m_data(control::DEFAULTS) {};

    //! Destructor
    ~Control() = default;

    //! Set all data in one step by deep-move of whole bundle
    void set(const control::Bundle& stack) { m_data = move(stack); }

    //! Set all flags in one step by deep-move of whole bool bundle
    void set(const control::BoolBundle& boolstack) {
      m_booldata = move(boolstack);
    }

    //! Get single element at position
    template< control::BundlePosition at >
    const typename std::tuple_element<at, decltype(m_data)>::type& get()
    const noexcept {
      return std::get<at>(m_data);
    }

    //! Check if an element is set during parse
    template< control::BundlePosition at >
    bool set() const { return m_booldata[at]; }

    //! Get physics keyword
    const std::string& physicsKeyword() const noexcept {
      return associate::PhysicsKeyword[ std::get<control::PHYSICS>(m_data) ];
    }
    //! Get physics name
    const std::string& physicsName() const noexcept {
      return associate::PhysicsName[ std::get<control::PHYSICS>(m_data) ];
    }

    //! Get hydrodynamics model keyword
    const std::string& hydroKeyword() const noexcept {
      return associate::HydroKeyword[ std::get<control::HYDRO>(m_data) ];
    }
    //! Get hydrodynamics model name
    const std::string& hydroName() const noexcept {
      return associate::HydroName[ std::get<control::HYDRO>(m_data) ];
    }

    //! Get material mix model keyword
    const std::string& mixKeyword() const noexcept {
      return associate::MixKeyword[ std::get<control::MIX>(m_data) ];
    }
    //! Get material mix model name
    const std::string& mixName() const noexcept {
      return associate::MixName[ std::get<control::MIX>(m_data) ];
    }

  private:
    //! Don't permit copy constructor
    Control(const Control&) = delete;
    //! Don't permit copy assigment
    Control& operator=(const Control&) = delete;
    //! Don't permit move constructor
    Control(Control&&) = delete;
    //! Don't permit move assigment
    Control& operator=(Control&&) = delete;
};

} // namespace Quinoa

#endif // Control_h
