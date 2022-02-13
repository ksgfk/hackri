#ifndef __HACKRI_MATH_UTIL_H__
#define __HACKRI_MATH_UTIL_H__

#include <limits>

namespace hackri {
constexpr auto PI_VALUE = 3.141592653589793;
template <class T>
constexpr T PI_V = static_cast<T>(PI_VALUE);
constexpr auto PI = PI_V<float>;
constexpr auto EPSILON = std::numeric_limits<float>::epsilon();

template <class T>
constexpr T Radian(T degree) noexcept { return degree / static_cast<T>(180.0) * PI_V<T>; }
template <class T>
constexpr T Degree(T radian) noexcept { return radian / PI_V<T> * static_cast<T>(180.0); }
template <class T>
constexpr T Lerp(T t, T v1, T v2) noexcept {
  return (static_cast<T>(1.0) - t) * v1 + t * v2;
}
}  // namespace hackri

#endif  // !__HACKRI_MATH_UTIL_H__