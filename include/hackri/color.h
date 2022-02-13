#ifndef __HACKRI_COLOR_H__
#define __HACKRI_COLOR_H__

#include <hackri/array.h>

#include <cstdint>

namespace hackri {
class Color3f;
class Color4f;
class Color3b;
class Color4b;

class Color3f : public Array<float, 3> {
 public:
  using BaseType = Array<float, 3>;

  constexpr Color3f() noexcept : BaseType() {}
  constexpr Color3f(float v) noexcept : BaseType(v) {}
  constexpr Color3f(float r, float g, float b) noexcept : BaseType(r, g, b) {}
  template <class T>
  constexpr Color3f(const Array<T, 3>& arr) : BaseType(arr) {}

  constexpr float& R() noexcept { return X(); }
  constexpr const float& R() const noexcept { return X(); }
  constexpr float& G() noexcept { return Y(); }
  constexpr const float& G() const noexcept { return Y(); }
  constexpr float& B() noexcept { return Z(); }
  constexpr const float& B() const noexcept { return Z(); }

  bool IsValid() const noexcept;
  Color3f ToLinear() const noexcept;
  Color3f ToSrgb() const noexcept;
  constexpr float Luminance() const noexcept {
    return R() * 0.212671f + G() * 0.715160f + B() * 0.072169f;
  }
  Color3b ToRGB() const noexcept;
};
class Color4f : public Array<float, 4> {
 public:
  using BaseType = Array<float, 4>;

  constexpr Color4f() noexcept : BaseType() {}
  constexpr Color4f(float v) noexcept : BaseType(v) {}
  constexpr Color4f(float r, float g, float b, float a) noexcept : BaseType(r, g, b, a) {}
  constexpr Color4f(const Color3f& rgb, float a) noexcept : Color4f(rgb.R(), rgb.G(), rgb.B(), a) {}
  template <class T>
  constexpr Color4f(const Array<T, 4>& arr) : BaseType(arr) {}

  constexpr float& R() noexcept { return X(); }
  constexpr const float& R() const noexcept { return X(); }
  constexpr float& G() noexcept { return Y(); }
  constexpr const float& G() const noexcept { return Y(); }
  constexpr float& B() noexcept { return Z(); }
  constexpr const float& B() const noexcept { return Z(); }
  constexpr float& A() noexcept { return W(); }
  constexpr const float& A() const noexcept { return W(); }

  bool IsValid() const noexcept;
  Color4b ToRGBA() const noexcept;
};
class Color3b : public Array<uint8_t, 3> {
 public:
  using BaseType = Array<uint8_t, 3>;

  constexpr Color3b() noexcept : BaseType() {}
  constexpr Color3b(uint8_t v) noexcept : BaseType(v) {}
  constexpr Color3b(uint8_t r, uint8_t g, uint8_t b) noexcept : BaseType(r, g, b) {}
  template <class T>
  constexpr Color3b(const Array<T, 3>& arr) : BaseType(arr) {}

  constexpr uint8_t& R() noexcept { return X(); }
  constexpr const uint8_t& R() const noexcept { return X(); }
  constexpr uint8_t& G() noexcept { return Y(); }
  constexpr const uint8_t& G() const noexcept { return Y(); }
  constexpr uint8_t& B() noexcept { return Z(); }
  constexpr const uint8_t& B() const noexcept { return Z(); }

  uint32_t ToInt32BGRA() const noexcept;
  uint32_t ToInt32RGBA() const noexcept;
};
class Color4b : public Array<uint8_t, 4> {
 public:
  using BaseType = Array<uint8_t, 4>;

  constexpr Color4b() noexcept : BaseType() {}
  constexpr Color4b(uint8_t v) noexcept : BaseType(v) {}
  constexpr Color4b(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept : BaseType(r, g, b, a) {}
  constexpr Color4b(const Color3b& rgb, uint8_t a) noexcept : Color4b(rgb.R(), rgb.G(), rgb.B(), a) {}
  template <class T>
  constexpr Color4b(const Array<T, 4>& arr) : BaseType(arr) {}

  constexpr uint8_t& R() noexcept { return X(); }
  constexpr const uint8_t& R() const noexcept { return X(); }
  constexpr uint8_t& G() noexcept { return Y(); }
  constexpr const uint8_t& G() const noexcept { return Y(); }
  constexpr uint8_t& B() noexcept { return Z(); }
  constexpr const uint8_t& B() const noexcept { return Z(); }
  constexpr uint8_t& A() noexcept { return W(); }
  constexpr const uint8_t& A() const noexcept { return W(); }

  uint32_t ToInt32BGRA() const noexcept;
  uint32_t ToInt32RGBA() const noexcept;
};
}  // namespace hackri

#endif