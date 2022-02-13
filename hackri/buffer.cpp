#include <hackri/buffer.h>

using namespace hackri;

ColorBuffer::ColorBuffer(uint32_t width, uint32_t height) noexcept
    : Buffer2d<Color4f>(width, height) {}
ColorBuffer::ColorBuffer(uint32_t width, uint32_t height, const Color4f* ptr) noexcept
    : Buffer2d<Color4f>(width, height, ptr) {}

DepthBuffer::DepthBuffer(uint32_t width, uint32_t height) noexcept
    : Buffer2d<float>(width, height) {}
DepthBuffer::DepthBuffer(uint32_t width, uint32_t height, const float* ptr) noexcept
    : Buffer2d<float>(width, height, ptr) {}