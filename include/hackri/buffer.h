#ifndef __HACKRI_BUFFER_H__
#define __HACKRI_BUFFER_H__

#include <hackri/color.h>
#include <vector>

namespace hackri {
template <class T>
class Buffer2d {
 public:
  Buffer2d(uint32_t width, uint32_t height) noexcept
      : _width(width), _height(height) {
    _buffer = std::vector<T>(size_t(width) * height, T());
  }
  Buffer2d(uint32_t width, uint32_t height, const T* ptr) noexcept
      : _width(width), _height(height) {
    auto length = size_t(width) * height;
    _buffer.reserve(length);
    std::copy(ptr, ptr + length, _buffer.begin());
  }

  constexpr uint32_t GetWidth() const noexcept { return _width; }
  constexpr uint32_t GetHeight() const noexcept { return _height; }
  T& operator()(uint32_t i, uint32_t j) {
    return _buffer[size_t(i) * GetHeight() + j];
  }
  const T& operator()(uint32_t i, uint32_t j) const {
    return _buffer[size_t(i) * GetHeight() + j];
  }

  void Fill(T value) noexcept {
    std::fill(_buffer.begin(), _buffer.end(), value);
  }

 private:
  uint32_t _width;
  uint32_t _height;
  std::vector<T> _buffer;
};

class ColorBuffer : public Buffer2d<Color4f> {
 public:
  ColorBuffer(uint32_t width, uint32_t height) noexcept;
  ColorBuffer(uint32_t width, uint32_t height, const Color4f* ptr) noexcept;
};
class DepthBuffer : public Buffer2d<float> {
 public:
  DepthBuffer(uint32_t width, uint32_t height) noexcept;
  DepthBuffer(uint32_t width, uint32_t height, const float* ptr) noexcept;
};
}  // namespace hackri

#endif