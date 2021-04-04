#ifndef CE_INCLUDE_CE_CONCEPTS_HPP
#define CE_INCLUDE_CE_CONCEPTS_HPP

#include "ce/cvector.hpp"
#include "ce/dvector.hpp"

namespace ce
{
  template <typename>
  constexpr inline bool is_cvector_v = false;

  template <typename T, int N>
  constexpr inline bool is_cvector_v<cvector<T, N>> = true;

  template <typename T>
  concept is_cvector = is_cvector_v<T>;

  template <typename T, int N>
  constexpr bool is_cvector_base_v(cvector<T, N>*) {
    return true;
  }

  constexpr bool is_cvector_base_v(...) {
    return false;
  }

  template <typename T>
  concept is_cvector_base = is_cvector_base_v((T*)nullptr);

  template <typename>
  constexpr inline bool is_dvector_v = false;

  template <typename T>
  constexpr inline bool is_dvector_v<dvector<T>> = true;

  template <typename T>
  concept is_dvector = is_dvector_v<T>;

  template <typename T>
  constexpr bool is_dvector_base_v(dvector<T>*) {
    return true;
  }

  constexpr bool is_dvector_base_v(...) {
    return false;
  }

  template <typename T>
  concept is_dvector_base = is_dvector_base_v((T*)nullptr);

  template <typename T>
  concept is_vector = is_cvector<T> || is_dvector<T>;

  template <typename T>
  concept is_vector_base = is_cvector_base<T> || is_dvector_base<T>;
}

#endif // CE_INCLUDE_CE_CONCEPTS_HPP
