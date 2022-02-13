#ifndef __HACKRI_IMAGE_H__
#define __HACKRI_IMAGE_H__

#include <cstdint>
#include <memory>

namespace hackri {
class Bitmap {
 public:
  struct Header {
    uint32_t biSize;
    uint32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
  };

  Bitmap(int width, int height);
  Bitmap(const char* filename);
  Bitmap(const Bitmap&);
  Bitmap(Bitmap&&);
  ~Bitmap();

  int GetW() const noexcept { return _w; }
  int GetH() const noexcept { return _h; }
  int GetPitch() const noexcept { return _pitch; }
  uint8_t* GetBits() noexcept;
  const uint8_t* GetBits() const noexcept;
  uint8_t* GetLine(int y) noexcept;
  const uint8_t* GetLine(int y) const noexcept;

  void Fill(uint32_t color);
  void SetPixel(int x, int y, uint32_t color);
  uint32_t GetPixel(int x, int y) const;
  bool SaveFile(const char* filename, bool withAlpha = false) const;
  uint32_t SampleBilinear(float x, float y) const;
  uint32_t Sample2D(float u, float v) const;
  void FlipVertical();
  void FlipHorizontal();

  static bool LoadFile(const char* filename, std::unique_ptr<uint8_t[]>& bits, int32_t& width, int32_t& height);
  static uint32_t BilinearInterp(uint32_t tl, uint32_t tr,
                                 uint32_t bl, uint32_t br,
                                 int32_t distx, int32_t disty);

 private:
  int32_t _w;
  int32_t _h;
  int32_t _pitch;
  std::unique_ptr<uint8_t[]> _bits;
};
}  // namespace hackri

#endif