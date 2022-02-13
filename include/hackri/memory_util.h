#ifndef __HACKRI_MEMORY_UTIL_H__
#define __HACKRI_MEMORY_UTIL_H__

#include <cstdint>
#include <type_traits>
#include <cassert>

namespace hackri {
template <class T>
constexpr T* OffsetByte(T* ptr, size_t offset) noexcept {
  return reinterpret_cast<T*>(reinterpret_cast<std::uint8_t*>(ptr) + offset);
}

template <class T>
class Span {
 public:
  constexpr Span() {
    _ptr = nullptr;
    _length = 0;
  }
  constexpr Span(T* array, size_t start, size_t length) {
    _ptr = array + start;
    _length = length;
  }
  constexpr Span(T* array, size_t length) : Span(array, 0, length) {}

  constexpr bool IsEmpty() const noexcept { return _ptr == nullptr || _length == 0; }

  constexpr size_t Length() const noexcept { return _length; }

  constexpr T& operator[](size_t i) {
    assert(i < _length);
    return _ptr[i];
  }

  constexpr const T& operator[](size_t i) const {
    assert(i < _length);
    return _ptr[i];
  }

  template <std::enable_if_t<std::is_trivially_destructible_v<T>, int> = 0>
  constexpr void Clear() {
    for (size_t i = 0; i < _length; i++) {
      _ptr[i].~T();
    }
  }

  constexpr void Fill(const T& value) {
    for (size_t i = 0; i < _length; i++) {
      _ptr[i] = value;
    }
  }

  template <std::enable_if_t<std::is_copy_assignable_v<T>, int> = 0>
  constexpr void CopyTo(Span<T>& dest) {
    for (size_t i = 0; i < _length; i++) {
      dest._ptr[i] = _ptr[i];
    }
  }

  constexpr Span<T> Slice(size_t start) const noexcept {
    assert(start < _length);
    return Span<T>(_ptr, start, _length - start);
  }

  constexpr Span<T> Slice(size_t start, size_t length) const noexcept {
    assert(start + length <= _length);
    return Span<T>(_ptr, start, length);
  }

  constexpr T* GetPointer() const noexcept { return _ptr; }

  template <class TTarget>
  constexpr Span<TTarget> Cast() const noexcept {
    return Span<TTarget>(reinterpret_cast<TTarget*>(_ptr), sizeof(T) * _length / sizeof(TTarget));
  }

 private:
  T* _ptr;
  size_t _length;
};
}  // namespace hackri

#endif