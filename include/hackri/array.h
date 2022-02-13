#ifndef __HACKRI_ARRAY_H__
#define __HACKRI_ARRAY_H__

#include <cmath>
#include <type_traits>
#include <array>
#include <algorithm>
#include <functional>
#include <ostream>
#include <initializer_list>

namespace hackri {
template <class T, size_t N>
class Array {
 public:
  using ThisType = Array<T, N>;
  using ContainerType = std::array<T, N>;
  using ValueType = typename ContainerType::value_type;
  using SizeType = typename ContainerType::size_type;
  using Reference = typename ContainerType::reference;
  using ConstReference = typename ContainerType::const_reference;
  using Iterator = typename ContainerType::iterator;
  using ConstIterator = typename ContainerType::const_iterator;

  constexpr static SizeType ContainerSize = N;

  constexpr Array() noexcept = default;
  constexpr explicit Array(T value) noexcept {
    for (size_t i = 0; i < N; i++) {
      _data[i] = value;
    }
  }
  constexpr Array(std::initializer_list<T> u) { std::copy(u.begin(), u.begin() + N, _data.begin()); }
  template <class TA, class TB, size_t _N = N, std::enable_if_t<_N == 2, int> = 0>
  constexpr Array(TA a, TB b) noexcept {
    _data[0] = a;
    _data[1] = b;
  }
  template <class TA, class TB, class TC, size_t _N = N, std::enable_if_t<_N == 3, int> = 0>
  constexpr Array(TA a, TB b, TC c) noexcept {
    _data[0] = a;
    _data[1] = b;
    _data[2] = c;
  }
  template <class TA, class TB, class TC, class TD, size_t _N = N, std::enable_if_t<_N == 4, int> = 0>
  constexpr Array(TA a, TB b, TC c, TD d) noexcept {
    _data[0] = a;
    _data[1] = b;
    _data[2] = c;
    _data[3] = d;
  }
  constexpr Array(const T* arr) noexcept { std::copy(arr, arr + N, _data.begin()); }

  constexpr size_t ElementCount() const noexcept { return _data.size(); }
  constexpr Reference operator[](size_t i) noexcept { return _data[i]; }
  constexpr ConstReference operator[](size_t i) const noexcept { return _data[i]; }
  constexpr Iterator Data() noexcept { return _data.data(); }
  constexpr ConstIterator Data() const noexcept { return _data.data(); }
  constexpr Iterator Begin() noexcept { return _data.begin(); }
  constexpr ConstIterator Begin() const noexcept { return _data.begin(); }
  constexpr Iterator End() noexcept { return _data.end(); }
  constexpr ConstIterator End() const noexcept { return _data.end(); }
  constexpr Reference Front() noexcept { return _data.front(); }
  constexpr ConstReference Front() const noexcept { return _data.front(); }
  constexpr Reference Back() noexcept { _data.back(); }
  constexpr ConstReference Back() const noexcept { return _data.back(); }
  constexpr void Fill(const ValueType& value) noexcept {
    for (size_t i = 0; i < N; i++) {
      _data[i] = value;
    }
  }
  constexpr void Swap(Array& other) noexcept { _data.swap(other._data); }

  template <size_t S = N, std::enable_if_t<(S >= 1), int> = 0>
  constexpr ConstReference X() const noexcept { return _data[0]; }
  template <size_t S = N, std::enable_if_t<(S >= 1), int> = 0>
  constexpr Reference X() noexcept { return _data[0]; }
  template <size_t S = N, std::enable_if_t<(S >= 2), int> = 0>
  constexpr ConstReference Y() const noexcept { return _data[1]; }
  template <size_t S = N, std::enable_if_t<(S >= 2), int> = 0>
  constexpr Reference Y() noexcept { return _data[1]; }
  template <size_t S = N, std::enable_if_t<(S >= 3), int> = 0>
  constexpr ConstReference Z() const noexcept { return _data[2]; }
  template <size_t S = N, std::enable_if_t<(S >= 3), int> = 0>
  constexpr Reference Z() noexcept { return _data[2]; }
  template <size_t S = N, std::enable_if_t<(S >= 4), int> = 0>
  constexpr ConstReference W() const noexcept { return _data[3]; }
  template <size_t S = N, std::enable_if_t<(S >= 4), int> = 0>
  constexpr Reference W() noexcept { return _data[3]; }

  constexpr ThisType& operator+=(const ThisType& rhs) noexcept { return *this = *this + rhs; }
  constexpr ThisType& operator-=(const ThisType& rhs) noexcept { return *this = *this - rhs; }
  constexpr ThisType& operator*=(const ThisType& rhs) noexcept { return *this = *this * rhs; }
  constexpr ThisType& operator/=(const ThisType& rhs) noexcept { return *this = *this / rhs; }
  constexpr ThisType& operator+=(T rhs) noexcept { return *this = *this + rhs; }
  constexpr ThisType& operator-=(T rhs) noexcept { return *this = *this - rhs; }
  constexpr ThisType& operator*=(T rhs) noexcept { return *this = *this * rhs; }
  constexpr ThisType& operator/=(T rhs) noexcept { return *this = *this / rhs; }

  constexpr T LengthSquared() const noexcept { return Dot(*this, *this); }
  constexpr T Length() const noexcept { return std::sqrt(LengthSquared()); }

  template <size_t S = N, std::enable_if_t<(S >= 2), int> = 0>
  constexpr Array<T, 2> XY() const noexcept { return Array<T, 2>(X(), Y()); }
  template <size_t S = N, std::enable_if_t<(S >= 3), int> = 0>
  constexpr Array<T, 3> XYZ() const noexcept { return Array<T, 3>(X(), Y(), Z()); }
  template <size_t S = N, std::enable_if_t<(S >= 3), int> = 0>
  constexpr Array<T, 4> XYZ1() const noexcept { return Array<T, 4>(X(), Y(), Z(), static_cast<T>(1.0)); }
  template <size_t S = N, std::enable_if_t<(S >= 3), int> = 0>
  constexpr Array<T, 4> XYZ0() const noexcept { return Array<T, 4>(X(), Y(), Z(), static_cast<T>(0.0)); }

  constexpr Array<T, N> ForeachFloor() const noexcept {
    Array<T, N> result;
    for (size_t i = 0; i < N; i++) {
      result[i] = std::floor((*this)[i]);
    }
    return result;
  }
  constexpr Array<T, N> ForeachCeil() const noexcept {
    Array<T, N> result;
    for (size_t i = 0; i < N; i++) {
      result[i] = std::ceil((*this)[i]);
    }
    return result;
  }
  constexpr Array<T, N> ForeachClamp(T minV, T maxV) const noexcept {
    Array<T, N> result;
    for (size_t i = 0; i < N; i++) {
      result[i] = std::clamp((*this)[i], minV, maxV);
    }
    return result;
  }
  template <class TTarget>
  constexpr Array<TTarget, N> Cast() const noexcept {
    Array<TTarget, N> result;
    for (size_t i = 0; i < N; i++) {
      result[i] = static_cast<TTarget>((*this)[i]);
    }
    return result;
  }

 private:
  ContainerType _data;
};
//############
//# 运算符重载 #
//############
template <class T, size_t N, class Operator>
constexpr Array<T, N> BinaryOperate(const Array<T, N>& lhs, const Array<T, N>& rhs, Operator&& op) noexcept {
  Array<T, N> result;
  std::transform(lhs.Begin(), lhs.End(), rhs.Begin(), result.Begin(), op);
  return result;
}
template <class T, size_t N, class Operator>
constexpr Array<T, N> BinaryOperate(const Array<T, N>& lhs, T rhs, Operator&& op) noexcept {
  Array<T, N> result;
  for (size_t i = 0; i < N; i++) {
    result[i] = op(lhs[i], rhs);
  }
  return result;
}
template <class T, size_t N>
constexpr bool operator==(const Array<T, N>& lhs, const Array<T, N>& rhs) noexcept {
  for (size_t i = 0; i < N; i++) {
    if (lhs[i] != rhs[i]) {
      return false;
    }
  }
  return true;
}
template <class T, size_t N>
constexpr bool operator!=(const Array<T, N>& lhs, const Array<T, N>& rhs) noexcept {
  return !(lhs == rhs);
}
template <class T, size_t N>
constexpr Array<T, N> operator+(const Array<T, N>& lhs, const Array<T, N>& rhs) noexcept {
  return BinaryOperate(lhs, rhs, std::plus<T>{});
}
template <class T, size_t N>
constexpr Array<T, N> operator-(const Array<T, N>& lhs, const Array<T, N>& rhs) noexcept {
  return BinaryOperate(lhs, rhs, std::minus<T>{});
}
template <class T, size_t N>
constexpr Array<T, N> operator*(const Array<T, N>& lhs, const Array<T, N>& rhs) noexcept {
  return BinaryOperate(lhs, rhs, std::multiplies<T>{});
}
template <class T, size_t N>
constexpr Array<T, N> operator/(const Array<T, N>& lhs, const Array<T, N>& rhs) noexcept {
  return BinaryOperate(lhs, rhs, std::divides<T>{});
}
template <class T, size_t N>
constexpr Array<T, N> operator+(const Array<T, N>& lhs, T rhs) noexcept {
  return BinaryOperate(lhs, rhs, std::plus<T>{});
}
template <class T, size_t N>
constexpr Array<T, N> operator-(const Array<T, N>& lhs, T rhs) noexcept {
  return BinaryOperate(lhs, rhs, std::minus<T>{});
}
template <class T, size_t N>
constexpr Array<T, N> operator*(const Array<T, N>& lhs, T rhs) noexcept {
  return BinaryOperate(lhs, rhs, std::multiplies<T>{});
}
template <class T, size_t N>
constexpr Array<T, N> operator/(const Array<T, N>& lhs, T rhs) noexcept {
  return BinaryOperate(lhs, rhs, std::divides<T>{});
}
template <class T, size_t N>
constexpr Array<T, N> operator+(T lhs, const Array<T, N>& rhs) noexcept {
  return BinaryOperate(rhs, lhs, std::plus<T>{});
}
template <class T, size_t N>
constexpr Array<T, N> operator*(T lhs, const Array<T, N>& rhs) noexcept {
  return BinaryOperate(rhs, lhs, std::multiplies<T>{});
}
template <class T, size_t N>
constexpr Array<T, N> operator-(const Array<T, N>& arr) noexcept {
  Array<T, N> result;
  for (size_t i = 0; i < N; i++) {
    result[i] = -arr[i];
  }
  return result;
}
template <class T, size_t N>
inline std::ostream& operator<<(std::ostream& out, const Array<T, N>& arr) {
  out << '<' << arr[0];
  for (size_t i = 1; i < N; i++) {
    out << ',' << arr[i];
  }
  out << '>';
  return out;
}
//##############
//# 一些扩展运算 #
//##############
template <class T, size_t N>
constexpr Array<T, N> Radian(const Array<T, N>& degree) noexcept {
  return degree / Array<T, N>(static_cast<T>(180.0)) * Array<T, N>(PI_V<T>);
}
template <class T, size_t N>
constexpr Array<T, N> Degree(const Array<T, N>& radian) noexcept {
  return radian / Array<T, N>(PI_V<T>) * Array<T, N>(static_cast<T>(180.0));
}
template <class T, size_t N>
constexpr Array<T, N> Lerp(const Array<T, N>& t, const Array<T, N>& v1, const Array<T, N>& v2) noexcept {
  return (Array<T, N>(static_cast<T>(1.0)) - t) * v1 + t * v2;
}
template <class T, class TScale, size_t N>
constexpr Array<T, N> Lerp(const TScale& scale, const Array<T, N>& v1, const Array<T, N>& v2) noexcept {
  Array<T, N> t(static_cast<T>(scale));
  return (Array<T, N>(static_cast<T>(1.0)) - t) * v1 + t * v2;
}
template <class T>
constexpr bool NearEqual(T u, T v, T epsilon) noexcept {
  return std::abs(u - v) <= epsilon;
}
template <class T, size_t N>
constexpr bool NearEqual(const Array<T, N>& u, const Array<T, N>& v, const Array<T, N>& epsilon) noexcept {
  for (size_t i = 0; i < N; i++) {
    if (std::abs(u[i] - v[i]) > epsilon[i]) {
      return false;
    }
  }
  return true;
}
template <class T, size_t N>
constexpr Array<T, N> SelectMin(const Array<T, N>& u, const Array<T, N>& v) noexcept {
  Array<T, N> result;
  for (size_t i = 0; i < N; i++) {
    result[i] = std::min(u[i], v[i]);
  }
  return result;
}
template <class T, size_t N>
constexpr Array<T, N> SelectMax(const Array<T, N>& u, const Array<T, N>& v) noexcept {
  Array<T, N> result;
  for (size_t i = 0; i < N; i++) {
    result[i] = std::max(u[i], v[i]);
  }
  return result;
}
//##############
//# 线性代数运算 #
//##############
template <class T, size_t N>
constexpr Array<T, N> Normalize(const Array<T, N>& u) noexcept {
  return u / u.Length();
}
template <class T, size_t N>
constexpr T Dot(const Array<T, N>& u, const Array<T, N>& v) noexcept {
  auto result = static_cast<T>(0.0);
  for (size_t i = 0; i < N; i++) {
    result += u[i] * v[i];
  }
  return result;
}
template <class T, size_t N>
constexpr T CosTheta(const Array<T, N>& u, const Array<T, N>& v) noexcept {
  return Dot(u, v) / (u.Length() * v.Length());
}
template <class T>
constexpr Array<T, 3> Cross(const Array<T, 3>& u, const Array<T, 3>& v) noexcept {
  return Array<T, 3>(u.Y() * v.Z() - u.Z() * v.Y(),
                     u.Z() * v.X() - u.X() * v.Z(),
                     u.X() * v.Y() - u.Y() * v.X());
}
template <class T>
constexpr T Cross(const Array<T, 2>& u, const Array<T, 2>& v) noexcept {
  return u.X() * v.Y() - u.Y() * v.X();
}
template <class T>
constexpr T SinPhi(const Array<T, 3>& u, const Array<T, 3>& v) noexcept {
  return Cross(u, v).Length() / (u.Length() * v.Length());
}
template <class T>
constexpr T SinPhi(const Array<T, 2>& u, const Array<T, 2>& v) noexcept {
  return Cross(u, v) / (u.Length() * v.Length());
}
template <class T, size_t N>  //向量u在向量v方向上的投影
constexpr Array<T, N> Project(const Array<T, N>& u, const Array<T, N>& v) noexcept {
  return v * (Dot(u, v) / v.LengthSquared());
}
template <class T, size_t N>  //如果把u向量分解成a1和a2，a1是v方向上的投影，a2就是Reject运算
constexpr Array<T, N> Reject(const Array<T, N>& u, const Array<T, N>& v) noexcept {
  return (u - v * Array<T, N>(Dot(u, v) / Dot(v, v)));
}
}  // namespace hackri

#endif  // !__HACKRI_ARRAY_H__
