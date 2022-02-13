#include <hackri/image.h>

#include <cstdio>
#include <cstdlib>
#include <string>
#include <stdexcept>
#include <algorithm>

using namespace hackri;

Bitmap::Bitmap(int width, int height) : _w(width), _h(height) {
  _pitch = width * 4;
  _bits = std::make_unique<uint8_t[]>(_pitch * _h);
  Fill(0);
}

Bitmap::Bitmap(const char* filename) {
  int32_t width;
  int32_t height;
  if (!LoadFile(filename, _bits, width, height)) {
    std::string msg = "load failed: ";
    msg.append(filename);
    throw std::runtime_error(msg);
  }
  _w = width;
  _h = height;
  _pitch = width * 4;
}

Bitmap::Bitmap(const Bitmap& src) : _w(src._w), _h(src._h), _pitch(src._pitch) {
  _bits = std::make_unique<uint8_t[]>(_pitch * _h);
  memcpy(_bits.get(), src._bits.get(), _pitch * _h);
}

Bitmap::Bitmap(Bitmap&& o) {
  _w = o._w;
  _h = o._h;
  _pitch = o._pitch;
  _bits = std::move(o._bits);
  o._bits = nullptr;
}

Bitmap::~Bitmap() = default;

uint8_t* Bitmap::GetBits() noexcept { return _bits.get(); }

const uint8_t* Bitmap::GetBits() const noexcept { return _bits.get(); }

uint8_t* Bitmap::GetLine(int y) noexcept { return GetBits() + _pitch * y; }

const uint8_t* Bitmap::GetLine(int y) const noexcept { return GetBits() + _pitch * y; }

void Bitmap::Fill(uint32_t color) {
  for (int j = 0; j < _h; j++) {
    uint32_t* row = (uint32_t*)(GetBits() + j * _pitch);
    for (int i = 0; i < _w; i++, row++)
      memcpy(row, &color, sizeof(uint32_t));
  }
}

void Bitmap::SetPixel(int x, int y, uint32_t color) {
  if (x >= 0 && x < _w && y >= 0 && y < _h) {
    memcpy(GetBits() + y * _pitch + x * 4, &color, sizeof(uint32_t));
  }
}

uint32_t Bitmap::GetPixel(int x, int y) const {
  uint32_t color = 0;
  if (x >= 0 && x < _w && y >= 0 && y < _h) {
    memcpy(&color, GetBits() + y * _pitch + x * 4, sizeof(uint32_t));
  }
  return color;
}

bool Bitmap::LoadFile(const char* filename,
                      std::unique_ptr<uint8_t[]>& bits,
                      int32_t& width, int32_t& height) {
  FILE* fp = fopen(filename, "rb");
  if (fp == nullptr) return false;
  Header info;
  uint8_t header[14];
  int hr = (int)fread(header, 1, 14, fp);
  if (hr != 14) {
    fclose(fp);
    return false;
  }
  if (header[0] != 0x42 || header[1] != 0x4d) {
    fclose(fp);
    return false;
  }
  hr = (int)fread(&info, 1, sizeof(info), fp);
  if (hr != 40) {
    fclose(fp);
    return false;
  }
  if (info.biBitCount != 24 && info.biBitCount != 32) {
    fclose(fp);
    return false;
  }
  bits = std::make_unique<uint8_t[]>(info.biWidth * info.biHeight * 4);
  width = info.biWidth;
  height = info.biHeight;
  uint32_t offset;
  memcpy(&offset, header + 10, sizeof(uint32_t));
  fseek(fp, offset, SEEK_SET);
  uint32_t pixelsize = (info.biBitCount + 7) / 8;
  uint32_t pitch = (pixelsize * info.biWidth + 3) & (~3);
  for (int y = 0; y < (int)info.biHeight; y++) {
    uint8_t* line = bits.get() + (info.biWidth * 4) * (info.biHeight - 1 - y);
    for (int x = 0; x < (int)info.biWidth; x++, line += 4) {
      line[3] = 255;
      fread(line, pixelsize, 1, fp);
    }
    fseek(fp, pitch - info.biWidth * pixelsize, SEEK_CUR);
  }
  fclose(fp);
  return true;
}

bool Bitmap::SaveFile(const char* filename, bool withAlpha) const {
  FILE* fp = fopen(filename, "wb");
  if (fp == NULL) return false;
  Header info;
  uint32_t pixelsize = (withAlpha) ? 4 : 3;
  uint32_t pitch = (GetW() * pixelsize + 3) & (~3);
  info.biSizeImage = pitch * GetH();
  uint32_t bfSize = 54 + info.biSizeImage;
  uint32_t zero = 0, offset = 54;
  fputc(0x42, fp);
  fputc(0x4d, fp);
  fwrite(&bfSize, 4, 1, fp);
  fwrite(&zero, 4, 1, fp);
  fwrite(&offset, 4, 1, fp);
  info.biSize = 40;
  info.biWidth = GetW();
  info.biHeight = GetH();
  info.biPlanes = 1;
  info.biBitCount = (withAlpha) ? 32 : 24;
  info.biCompression = 0;
  info.biXPelsPerMeter = 0xb12;
  info.biYPelsPerMeter = 0xb12;
  info.biClrUsed = 0;
  info.biClrImportant = 0;
  fwrite(&info, sizeof(info), 1, fp);
  for (int y = 0; y < GetH(); y++) {
    const uint8_t* line = GetLine(info.biHeight - 1 - y);
    uint32_t padding = pitch - GetW() * pixelsize;
    for (int x = 0; x < GetW(); x++, line += 4) {
      fwrite(line, pixelsize, 1, fp);
    }
    for (int i = 0; i < (int)padding; i++) fputc(0, fp);
  }
  fclose(fp);
  return true;
}

uint32_t Bitmap::SampleBilinear(float x, float y) const {
  int32_t fx = (int32_t)(x * 0x10000);
  int32_t fy = (int32_t)(y * 0x10000);
  int32_t x1 = std::clamp(fx >> 16, 0, _w - 1);
  int32_t y1 = std::clamp(fy >> 16, 0, _h - 1);
  int32_t x2 = std::clamp(x1 + 1, 0, _w - 1);
  int32_t y2 = std::clamp(y1 + 1, 0, _h - 1);
  int32_t dx = (fx >> 8) & 0xff;
  int32_t dy = (fy >> 8) & 0xff;
  if (_w <= 0 || _h <= 0) return 0;
  uint32_t c00 = GetPixel(x1, y1);
  uint32_t c01 = GetPixel(x2, y1);
  uint32_t c10 = GetPixel(x1, y2);
  uint32_t c11 = GetPixel(x2, y2);
  return BilinearInterp(c00, c01, c10, c11, dx, dy);
}

uint32_t Bitmap::Sample2D(float u, float v) const {
  uint32_t rgba = SampleBilinear(u * _w + 0.5f, v * _h + 0.5f);
  return rgba;
}

void Bitmap::FlipVertical() {
  std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(_pitch);
  for (int i = 0, j = _h - 1; i < j; i++, j--) {
    memcpy(buffer.get(), GetLine(i), _pitch);
    memcpy(GetLine(i), GetLine(j), _pitch);
    memcpy(GetLine(j), buffer.get(), _pitch);
  }
}

void Bitmap::FlipHorizontal() {
  for (int y = 0; y < _h; y++) {
    for (int i = 0, j = _w - 1; i < j; i++, j--) {
      uint32_t c1 = GetPixel(i, y);
      uint32_t c2 = GetPixel(j, y);
      SetPixel(i, y, c2);
      SetPixel(j, y, c1);
    }
  }
}

uint32_t Bitmap::BilinearInterp(uint32_t tl, uint32_t tr,
                                uint32_t bl, uint32_t br, int32_t distx, int32_t disty) {
  uint32_t f, r;
  int32_t distxy = distx * disty;
  int32_t distxiy = (distx << 8) - distxy; /* distx * (256 - disty) */
  int32_t distixy = (disty << 8) - distxy; /* disty * (256 - distx) */
  int32_t distixiy = 256 * 256 - (disty << 8) - (distx << 8) + distxy;
  r = (tl & 0x000000ff) * distixiy + (tr & 0x000000ff) * distxiy + (bl & 0x000000ff) * distixy + (br & 0x000000ff) * distxy;
  f = (tl & 0x0000ff00) * distixiy + (tr & 0x0000ff00) * distxiy + (bl & 0x0000ff00) * distixy + (br & 0x0000ff00) * distxy;
  r |= f & 0xff000000;
  tl >>= 16;
  tr >>= 16;
  bl >>= 16;
  br >>= 16;
  r >>= 16;
  f = (tl & 0x000000ff) * distixiy + (tr & 0x000000ff) * distxiy + (bl & 0x000000ff) * distixy + (br & 0x000000ff) * distxy;
  r |= f & 0x00ff0000;
  f = (tl & 0x0000ff00) * distixiy + (tr & 0x0000ff00) * distxiy + (bl & 0x0000ff00) * distixy + (br & 0x0000ff00) * distxy;
  r |= f & 0xff000000;
  return r;
}