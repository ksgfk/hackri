#ifndef __HACKRI_MATRIX_H__
#define __HACKRI_MATRIX_H__

#include "array.h"
#include <initializer_list>
#include <utility>
#include <optional>

namespace hackri {
//00 01 02 03
//10 11 12 13
//20 21 22 23
//30 31 32 33
template <class T, size_t R, size_t C>
class Matrix {
  static_assert(R > 0 && C > 0, "row or column can't be zero");

 public:
  using DataType = T;
  using PacketType = Array<DataType, C>;
  using ContainerType = Array<PacketType, R>;
  using RawType = T[R * C];
  union UnionType {
    ContainerType Use;
    RawType Raw;
  };

  constexpr static size_t ContainerSize = R;

  constexpr Matrix() noexcept = default;
  constexpr Matrix(std::initializer_list<DataType> param) noexcept {
    std::copy(param.begin(), param.begin() + R * C, _data.Raw);
  }
  constexpr Matrix(std::initializer_list<PacketType> param) noexcept {
    std::copy(param.begin(), param.begin() + ContainerSize, _data.Use.Begin());
  }

  constexpr PacketType& operator[](size_t i) noexcept { return _data.Use[i]; }
  constexpr const PacketType& operator[](size_t i) const noexcept { return _data.Use[i]; }
  constexpr DataType& operator()(size_t i, size_t j) noexcept { return _data.Use[i][j]; }
  constexpr const DataType& operator()(size_t i, size_t j) const noexcept { return _data.Use[i][j]; }

  constexpr PacketType GetRow(size_t row) noexcept {
    return _data.Use[row];
  }
  constexpr Array<T, R> GetColumn(size_t column) noexcept {
    Array<T, R> result;
    for (size_t i = 0; i < R; i++) {
      result[i] = _data.Use[i][column];
    }
    return result;
  }
  constexpr void SetRow(size_t row, const PacketType& arr) noexcept { _data.Use[row] = arr; }
  constexpr void SetColumn(size_t column, const Array<T, R>& arr) noexcept {
    for (size_t i = 0; i < R; i++) {
      m[i][column] = arr[i];
    }
  }

  template <size_t _R = R, size_t _C = C, std::enable_if_t<_R == _C, int> = 0>
  constexpr static Matrix<T, R, C> Identity() {
    Matrix<T, R, C> identify{};
    for (size_t i = 0; i < _R; i++) {
      identify(i, i) = static_cast<T>(1.0);
    }
    return identify;
  }
  constexpr static Matrix<T, R, C> Zero() {
    Matrix<T, R, C> zero;
    for (size_t r = 0; r < R; r++) {
      for (size_t c = 0; c < C; c++) {
        zero[r][c] = 0;
      }
    }
    return zero;
  }

  //转置
  constexpr Matrix<T, C, R> Transpose() const noexcept {
    auto x = Matrix<T, C, R>::ContainerSize;
    auto y = Matrix<T, C, R>::PacketType::ContainerSize;
    Matrix<T, C, R> result;
    for (size_t i = 0; i < x; i++) {
      for (size_t j = 0; j < y; j++) {
        result(i, j) = (*this)(j, i);
      }
    }
    return result;
  }
  //余子阵
  constexpr Matrix<T, R - 1, C - 1> Minor(size_t i, size_t j) const noexcept {
    Matrix<T, R - 1, C - 1> result;
    for (size_t r = 0; r < R - 1; r++) {
      for (size_t c = 0; c < C - 1; c++) {
        result[r][c] = (*this)[r < i ? r : r + 1][c < j ? c : c + 1];
      }
    }
    return result;
  }
  //矩阵的行列式
  template <size_t _R = R, size_t _C = C, std::enable_if_t<_R == _C && _R == 1, int> = 0>
  constexpr T Determinant() const noexcept {
    return (*this)(0, 0);
  }
  template <size_t _R = R, size_t _C = C, std::enable_if_t<_R == _C && _R == 2, int> = 0>
  constexpr T Determinant() const noexcept {
    const auto& r = *this;
    return r(0, 0) * r(1, 1) - r(0, 1) * r(1, 0);
  }
  template <size_t _R = R, size_t _C = C, std::enable_if_t<_R == _C && _R != 1 && _R != 2, int> = 0>
  constexpr T Determinant() const noexcept {
    const auto& r = *this;
    T result = static_cast<T>(0.0);
    for (size_t i = 0; i < R; i++) {
      result += r[0][i] * Cofactor(0, i);
    }
    return result;
  }
  //代数余子式
  template <size_t _R = R, size_t _C = C, std::enable_if_t<_R == _C && _R == 1, int> = 0>
  constexpr T Cofactor(size_t i, size_t j) const noexcept {
    return static_cast<T>(0.0);
  }
  template <size_t _R = R, size_t _C = C, std::enable_if_t<_R == _C && _R != 1, int> = 0>
  constexpr T Cofactor(size_t i, size_t j) const noexcept {
    return Minor(i, j).Determinant() * (((i + j) % 2) ? -1 : 1);
  }
  //代数余子式矩阵
  template <size_t _R = R, size_t _C = C, std::enable_if_t<_R == _C, int> = 0>
  constexpr Matrix<T, R, C> Cofactor() const noexcept {
    Matrix<T, R, C> result;
    for (size_t i = 0; i < R; i++) {
      for (size_t j = 0; j < C; j++) {
        result(i, j) = Cofactor(i, j);
      }
    }
    return result;
  }
  //伴随矩阵
  template <size_t _R = R, size_t _C = C, std::enable_if_t<_R == _C, int> = 0>
  constexpr Matrix<T, R, C> Adjoint() const noexcept {
    return Cofactor().Transpose();
  }
  //逆矩阵
  template <size_t _R = R, size_t _C = C, std::enable_if_t<_R == _C, int> = 0>
  constexpr std::optional<Matrix<T, R, C>> Invert() const noexcept {
    auto det = Determinant();
    if (det == 0.0f) {
      return std::nullopt;
    }
    return Adjoint() / det;
  }

 private:
  //[00 01 02 03][10 11 12 13][20 21 22 23][30 31 32 33]
  UnionType _data;
};
//############
//# 运算符重载 #
//############
template <class T, size_t LR, size_t LC, size_t RR, size_t RC, std::enable_if_t<(LC == RR), int> = 0>
constexpr Matrix<T, LR, RC> Multiply(const Matrix<T, LR, LC>& lhs, const Matrix<T, RR, RC>& rhs) noexcept {
  Matrix<T, LR, RC> result;
  for (size_t i = 0; i < LR; i++) {
    for (size_t j = 0; j < RC; j++) {
      auto t = static_cast<T>(0.0);
      for (size_t k = 0; k < LC; k++) {
        t += lhs(i, k) * rhs(k, j);
      }
      result(i, j) = t;
    }
  }
  return result;
}
template <class T, size_t N>
constexpr Array<T, N> Multiply(const Matrix<T, N, N>& lhs, const Array<T, N>& rhs) noexcept {
  Array<T, N> result;
  for (size_t i = 0; i < N; i++) {
    result[i] = Dot(lhs[i], rhs);
  }
  return result;
}
template <class T, size_t R, size_t C>
constexpr bool operator==(const Matrix<T, R, C>& lhs, const Matrix<T, R, C>& rhs) noexcept {
  auto x = Matrix<T, R, C>::ContainerSize;
  for (size_t i = 0; i < x; i++) {
    if (lhs[i] != rhs[i]) {
      return false;
    }
  }
  return true;
}
template <class T, size_t R, size_t C>
constexpr bool operator!=(const Matrix<T, R, C>& lhs, const Matrix<T, R, C>& rhs) noexcept {
  return !(lhs == rhs);
}
template <class T, size_t LR, size_t LC, size_t RR, size_t RC, std::enable_if_t<(LC == RR), int> = 0>
constexpr Matrix<T, LR, RC> operator*(const Matrix<T, LR, LC>& lhs, const Matrix<T, RR, RC>& rhs) noexcept {
  return Multiply(lhs, rhs);
}
template <class T, size_t N>
constexpr Array<T, N> operator*(const Matrix<T, N, N>& lhs, const Array<T, N>& rhs) noexcept {
  return Multiply(lhs, rhs);
}
template <class T, size_t R, size_t C>
constexpr Matrix<T, R, C> operator*(const Matrix<T, R, C>& lhs, T rhs) noexcept {
  auto x = Matrix<T, R, C>::ContainerSize;
  Matrix<T, R, C> result;
  for (size_t i = 0; i < x; i++) {
    result[i] = lhs[i] * rhs;
  }
  return result;
}
template <class T, size_t R, size_t C>
constexpr Matrix<T, R, C> operator/(const Matrix<T, R, C>& lhs, T rhs) noexcept {
  auto x = Matrix<T, R, C>::ContainerSize;
  Matrix<T, R, C> result;
  for (size_t i = 0; i < x; i++) {
    result[i] = lhs[i] / rhs;
  }
  return result;
}
template <class T, size_t R, size_t C>
constexpr Matrix<T, R, C> operator*(T lhs, const Matrix<T, R, C>& rhs) noexcept {
  return rhs * lhs;
}
template <class T, size_t R, size_t C>
constexpr Matrix<T, R, C> operator+(const Matrix<T, R, C>& lhs, const Matrix<T, R, C>& rhs) noexcept {
  auto x = Matrix<T, R, C>::ContainerSize;
  Matrix<T, R, C> result;
  for (size_t i = 0; i < x; i++) {
    result[i] = lhs[i] * rhs[i];
  }
  return result;
}
template <class T, size_t R, size_t C>
constexpr Matrix<T, R, C> operator-(const Matrix<T, R, C>& lhs, const Matrix<T, R, C>& rhs) noexcept {
  auto x = Matrix<T, R, C>::ContainerSize;
  Matrix<T, R, C> result;
  for (size_t i = 0; i < x; i++) {
    result[i] = lhs[i] - rhs[i];
  }
  return result;
}
template <class T, size_t R, size_t C>
inline std::ostream& operator<<(std::ostream& out, const Matrix<T, R, C>& mat) {
  auto x = Matrix<T, R, C>::ContainerSize;
  out << '[' << mat[0];
  for (size_t i = 1; i < x; i++) {
    out << ',' << mat[i];
  }
  out << ']';
  return out;
}
//###########
//# 常用矩阵 #
//###########
template <class T>
constexpr Matrix<T, 4, 4> Translate(const Array<T, 3>& vec) noexcept {
  auto result = Matrix<T, 4, 4>::Identity();
  result(0, 3) = vec[0];
  result(1, 3) = vec[1];
  result(2, 3) = vec[2];
  return result;
}
template <class T>
constexpr Matrix<T, 4, 4> Scale(const Array<T, 3>& vec) noexcept {
  Matrix<T, 4, 4> result{};
  result(0, 0) = vec[0];
  result(1, 1) = vec[1];
  result(2, 2) = vec[2];
  return result;
}
template <class T>
constexpr Matrix<T, 4, 4> AxisAngle(const Array<T, 3>& axis, T angle) noexcept {
  auto s = std::sin(angle);
  auto c = std::cos(angle);
  auto t = static_cast<T>(1) - c;
  auto n = Normalize(axis);
  return Matrix<T, 4, 4>(
      {t * n.X() * n.X() + c, t * n.X() * n.Y() - n.Z() * s, t * n.X() * n.Z() + n.Y() * s, static_cast<T>(0.0),
       t * n.X() * n.Y() + n.Z() * s, t * n.Y() * n.Y() + c, t * n.Y() * n.Z() - n.X() * s, static_cast<T>(0.0),
       t * n.X() * n.Z() - n.Y() * s, t * n.Y() * n.Z() + n.X() * s, t * n.Z() * n.Z() + c, static_cast<T>(0.0),
       static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(1.0)});
}
//假设：
//* 相机位于坐标原点
//* 相机看向z方向
//* 相机以y方向为上方向
template <class T>
constexpr Matrix<T, 4, 4> LookAt(const Array<T, 3>& origin, const Array<T, 3>& target, const Array<T, 3>& up) noexcept {
  auto z = Normalize(origin - target);
  auto x = Normalize(Cross(up, z));
  auto y = Cross(z, x);
  return Matrix<T, 4, 4>(
      {x.X(), x.Y(), x.Z(), -Dot(x, origin),
       y.X(), y.Y(), y.Z(), -Dot(y, origin),
       z.X(), z.Y(), z.Z(), -Dot(z, origin),
       static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(1.0)});
}
//由于相机的假设，near实际值大于0，且小于far
template <class T>
constexpr Matrix<T, 4, 4> Perspective(T fov, T aspect, T near, T far) noexcept {
  T halfTan = std::tan(fov * static_cast<T>(0.5));
  return Matrix<T, 4, 4>(
      {static_cast<T>(1.0) / (halfTan * aspect), static_cast<T>(0), static_cast<T>(0), static_cast<T>(0),
       static_cast<T>(0), static_cast<T>(1.0) / halfTan, static_cast<T>(0), static_cast<T>(0),
       static_cast<T>(0), static_cast<T>(0), -(near + far) / (far - near), -(static_cast<T>(2.0) * near * far) / (far - near),
       static_cast<T>(0), static_cast<T>(0), static_cast<T>(-1), static_cast<T>(0)});
}
}  // namespace hackri

#endif
