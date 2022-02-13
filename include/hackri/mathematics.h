#ifndef __HACKRI_MATHEMATICS_H__
#define __HACKRI_MATHEMATICS_H__

#include <hackri/math_util.h>
#include <hackri/array.h>
#include <hackri/matrix.h>
#include <hackri/color.h>

namespace hackri {
template <class T, size_t N>
using Vector = Array<T, N>;
using Vector2f = Vector<float, 2>;
using Vector3f = Vector<float, 3>;
using Vector4f = Vector<float, 4>;
using Matrix3f = Matrix<float, 3, 3>;
using Matrix4f = Matrix<float, 4, 4>;
}  // namespace hackri

#endif  // !__HACKRI_MATHEMATICS_H__
