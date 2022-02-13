#include <hackri/color.h>

#include <algorithm>

using namespace hackri;

bool Color3f::IsValid() const noexcept {
  for (size_t i = 0; i < 3; i++) {
    if (!std::isfinite((*this)[i]) || std::isnan((*this)[i])) {
      return false;
    }
  }
  return true;
}
Color3f Color3f::ToLinear() const noexcept {
  Color3f c;
  for (size_t i = 0; i < 3; i++) {
    c[i] = (*this)[i] <= 0.04045f
               ? (*this)[i] * (1.0f / 12.92f)
               : std::pow(((*this)[i] + 0.055f) * (1.0f / 1.055f), 2.4f);
  }
  return c;
}
Color3f Color3f::ToSrgb() const noexcept {
  Color3f c;
  for (size_t i = 0; i < 3; i++) {
    c[i] = (*this)[i] <= 0.0031308f
               ? 12.92f * (*this)[i]
               : (1.0f + 0.055f) * std::pow((*this)[i], 1.0f / 2.4f) - 0.055f;
  }
  return c;
}
Color3b Color3f::ToRGB() const noexcept {
  uint8_t r = (uint8_t)std::clamp((int)(R() * 255.0f), 0, 255);
  uint8_t g = (uint8_t)std::clamp((int)(G() * 255.0f), 0, 255);
  uint8_t b = (uint8_t)std::clamp((int)(B() * 255.0f), 0, 255);
  return Color3b{r, g, b};
}
bool Color4f::IsValid() const noexcept {
  for (size_t i = 0; i < 4; i++) {
    if (!std::isfinite((*this)[i]) || std::isnan((*this)[i])) {
      return false;
    }
  }
  return true;
}
Color4b Color4f::ToRGBA() const noexcept {
  uint8_t r = (uint8_t)std::clamp((int)(R() * 255.0f), 0, 255);
  uint8_t g = (uint8_t)std::clamp((int)(G() * 255.0f), 0, 255);
  uint8_t b = (uint8_t)std::clamp((int)(B() * 255.0f), 0, 255);
  uint8_t a = (uint8_t)std::clamp((int)(A() * 255.0f), 0, 255);
  return Color4b{r, g, b, a};
}
uint32_t Color3b::ToInt32BGRA() const noexcept {
  uint32_t r = R();
  uint32_t g = G();
  uint32_t b = B();
  uint32_t a = 1;
  return (r << 16) | (g << 8) | b | (a << 24);
}
uint32_t Color3b::ToInt32RGBA() const noexcept {
  uint32_t r = R();
  uint32_t g = G();
  uint32_t b = B();
  uint32_t a = 1;
  return (b << 16) | (g << 8) | r | (a << 24);
}
uint32_t Color4b::ToInt32BGRA() const noexcept {
  uint32_t r = R();
  uint32_t g = G();
  uint32_t b = B();
  uint32_t a = A();
  return (r << 16) | (g << 8) | b | (a << 24);
}
uint32_t Color4b::ToInt32RGBA() const noexcept {
  uint32_t r = R();
  uint32_t g = G();
  uint32_t b = B();
  uint32_t a = A();
  return (b << 16) | (g << 8) | r | (a << 24);
}
