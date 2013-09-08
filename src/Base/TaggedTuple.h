//******************************************************************************
/*!
  \file      src/Base/TaggedTuple.h
  \author    J. Bakosi
  \date      Sun 08 Sep 2013 04:28:54 PM MDT
  \copyright Copyright 2005-2012, Jozsef Bakosi, All rights reserved.
  \brief     Tagged tuple allowing tag-based access
  \details   Tagged tuple allowing tag-based access, credit goes to
             ecatmur@stackoverflow.com, for more details, see
             http://stackoverflow.com/questions/13065166/c11-tagged-tuple
*/
//******************************************************************************
#ifndef TaggedTuple_h
#define TaggedTuple_h

#include <tuple>

namespace quinoa {

//! Tagged tuple

//! typelist
template<typename... Ts> struct typelist {
  template<typename T> using prepend = typelist<T, Ts...>;
};

//! index implementation
template<typename T, typename... Ts> struct index;
template<typename T, typename... Ts> struct index<T, T, Ts...> :
  std::integral_constant<int, 0> {};
template<typename T, typename U, typename... Ts> struct index<T, U, Ts...> :
  std::integral_constant<int, index<T, Ts...>::value + 1> {};

//! nth type implementation
template<int n, typename... Ts> struct nth_impl;
template<typename T, typename... Ts> struct nth_impl<0, T, Ts...> {
  using type = T; };
template<int n, typename T, typename... Ts> struct nth_impl<n, T, Ts...> {
  using type = typename nth_impl<n - 1, Ts...>::type; };
template<int n, typename... Ts> using nth = typename nth_impl<n, Ts...>::type;

//! exctract implementation
template<int n, int m, typename... Ts> struct extract_impl;
template<int n, int m, typename T, typename... Ts>
struct extract_impl<n, m, T, Ts...> : extract_impl<n, m - 1, Ts...> {};
template<int n, typename T, typename... Ts>
struct extract_impl<n, 0, T, Ts...> { using types = typename
  extract_impl<n, n - 1, Ts...>::types::template prepend<T>; };
template<int n, int m> struct extract_impl<n, m> {
  using types = typelist<>; };
template<int n, int m, typename... Ts>
  using extract = typename extract_impl<n, m, Ts...>::types;

//! tagged_tuple implementation
template<typename S, typename T> struct tt_impl;
template<typename... Ss, typename... Ts>
struct tt_impl<typelist<Ss...>, typelist<Ts...>> : public std::tuple<Ts...> {
  //! Accessor to type of nth element
  template<typename S>
  using nT = nth<index<S, Ss...>::value, Ts...>;
  //! Constructor
  template<typename... Args> tt_impl(Args &&...args) :
    std::tuple<Ts...>(std::forward<Args>(args)...) {}
  //! Copy-value accessor
  //template<typename S> nth<index<S, Ss...>::value, Ts...> get() {
  //  return std::get<index<S, Ss...>::value>(*this); }
  //! Const-ref accessor
  template<typename S> constexpr const nT<S>& get() const {
    return std::get<index<S, Ss...>::value>(*this); }
  //! Rvalue accessor
  template<typename S> nT<S>& get() {
    return std::get<index<S, Ss...>::value>(*this); }
};

//! tagged_tuple
template<typename... Ts> struct tagged_tuple :
  tt_impl<extract<2, 0, Ts...>, extract<2, 1, Ts...>> {
  //! Constructor
  template<typename... Args> tagged_tuple(Args &&...args) :
    tt_impl<extract<2, 0, Ts...>, extract<2, 1, Ts...>>(
      std::forward<Args>(args)...) {}
};

// Find the size of a given tagged tuple type: tuple_size/2
template<typename _Tp>
  struct tagged_tuple_size;

template<typename _Tp>
  struct tagged_tuple_size<const _Tp> :
    public std::integral_constant<
           typename std::remove_cv<decltype(std::tuple_size<_Tp>::value)>::type,
           std::tuple_size<_Tp>::value> { };

template<typename _Tp>
  struct tagged_tuple_size<volatile _Tp> :
    public std::integral_constant<
           typename std::remove_cv<decltype(std::tuple_size<_Tp>::value)>::type,
           std::tuple_size<_Tp>::value> { };

template<typename _Tp>
  struct tagged_tuple_size<const volatile _Tp> :
    public std::integral_constant<
           typename std::remove_cv<decltype(std::tuple_size<_Tp>::value)>::type,
           std::tuple_size<_Tp>::value> { };

// Class tagged_tuple_size
template<typename... _Elements>
  struct tagged_tuple_size<tagged_tuple<_Elements...>> :
    public std::integral_constant<std::size_t, sizeof...(_Elements)/2> { };

} // namespace quinoa

#endif // TaggedTuple_h
