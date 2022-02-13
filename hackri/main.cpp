#include <hackri/mathematics.h>
#include <hackri/buffer.h>
#include <hackri/renderer.h>
#include <hackri/image.h>
#include <hackri/model.h>
#include <iostream>
#include <random>

using namespace std;
using namespace hackri;

struct Vertex {
  Vector3f Pos;
  Color3f Color;
};

int main() {
  auto model = ImmutableModel::CreateSphere(2.0f, 64);

  const int width = 1280;
  const int height = 720;

  Vertex v[] = {
      {{-1.25f, 0.75f, 0.0f}, {1.0f, 0.0f, 0.0f}},
      {{0.70f, 1.25f, 0.0f}, {0.0f, 1.0f, 0.0f}},
      {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}};
  Vertex v2[] = {
      {{-1.25f, 0.75f, 0.0f}, {1.0f, 0.0f, 0.0f}},
      {{0.70f, 1.25f, 0.0f}, {0.0f, 1.0f, 0.0f}},
      {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

  auto viewMat = LookAt(Vector3f{0.0f, 0.0f, 0.0f}, {0, 0.0f, -0.5f}, {0, 1, 0});
  auto persMat = Perspective(Radian(60.0f), (float)width / height, 0.001f, 10.0f);

  auto vs = [&](int index, VertexShaderParams& param) -> Vector4f {
    const Vertex& v = param.CastVertex<Vertex>()[index];
    Color3f& out = param.CastOut<Color3f>(index);
    out = v.Color;
    Vector4f holo = v.Pos.XYZ1();
    Matrix4f vp = persMat * viewMat;
    Vector4f r = vp * holo;
    return r;
  };
  auto ps = [](PixelShaderParams& param) -> Color4f {
    const Color3f& out = param.CastIn<Color3f>();
    return out.XYZ1();
  };

  ColorBuffer cb(width, height);
  DepthBuffer db(width, height);
  std::pmr::monotonic_buffer_resource buffer(16384);
  Bitmap img(width, height);

  PipelineState pso;
  pso.VS = vs;
  pso.PS = ps;
  pso.VertexSize = sizeof(Vertex);
  pso.OutLayout = {sizeof(Color3f)};
  pso.CBufferSize = 0;
  pso.IsDrawFrame = false;
  pso.DepthFunc = DepthComparison::Less;
  pso.Cull = CullMode::None;
  pso.FrontOrder = FrontFace::CCW;
  PipelineInput input;
  input.Vertex = reinterpret_cast<uint8_t*>(v);
  input.CBuffer = nullptr;
  input.ColorBuffer = &cb;
  input.DepthBuffer = &db;
  input.FrameWidth = width;
  input.FrameHeight = height;
  PipelineMemory memory;
  memory.Arena = &buffer;

  cb.Fill({0.0f, 0.0f, 0.0f, 1.0f});
  db.Fill(1.0f);
  Renderer::DrawTriangle(input, pso, memory);
  for (size_t i = 0; i < model.GetIndexCount(); i += 3) {
    for (size_t j = 0; j < 3; j++) {
      v[j].Pos = model.GetPositions()[model.GetIndices()[i + j]];
    }
    Renderer::DrawTriangle(input, pso, memory);
    buffer.release();
  }

  for (uint32_t x = 0; x < width; x++) {
    for (uint32_t y = 0; y < height; y++) {
      uint32_t c = cb(x, y).ToRGBA().ToInt32BGRA();
      img.SetPixel(x, y, c);
    }
  }
  img.FlipVertical();
  std::string path("C:\\Users\\ksgfk\\Desktop\\res.bmp");
  img.SaveFile(path.c_str(), false);

  return 0;
}