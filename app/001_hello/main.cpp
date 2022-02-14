#include <hackri/mathematics.h>
#include <hackri/buffer.h>
#include <hackri/renderer.h>
#include <hackri/image.h>
#include <hackri/model.h>
#include <iostream>

using namespace std;
using namespace hackri;

struct VertexVC {
  Vector3f Pos;
  Color3f Color;
};

struct VertexVN {
  Vector3f Pos;
  Vector3f Nor;
};

struct CBuffer {
  Matrix4f mvp;
};

constexpr int width = 1280;
constexpr int height = 720;
VertexVC helloWorld[] = {
    {{0.0f, 0.75f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{-0.5f, -0.75f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, -0.75f, 0.0f}, {0.0f, 0.0f, 1.0f}}};
CBuffer mvpBuffer;
int main() {
  ColorBuffer cb(width, height);
  DepthBuffer db(width, height);
  Bitmap img(width, height);
  std::pmr::monotonic_buffer_resource buffer(16384);
  auto saveResult = [&](std::string_view path) -> void {
    for (uint32_t x = 0; x < width; x++) {
      for (uint32_t y = 0; y < height; y++) {
        uint32_t c = cb(x, y).ToRGBA().ToInt32BGRA();
        img.SetPixel(x, y, c);
      }
    }
    img.FlipVertical();
    img.SaveFile(path.data(), false);
  };

  {
    auto vsHello = [](int index, VertexShaderParams& param) -> Vector4f {
      const VertexVC& v = param.CastVertex<VertexVC>()[index];
      Color3f& out = param.CastOut<Color3f>(index);
      out = v.Color;
      return v.Pos.XYZ1();
    };
    auto psHello = [](const PixelShaderParams& param, bool& isDiscard) -> Color4f {
      const Color3f& out = param.CastIn<Color3f>();
      return out.XYZ1();
    };

    PipelineState pso = Renderer::DefaultPSO(vsHello, psHello, sizeof(VertexVC), sizeof(Color3f));
    PipelineInput input;
    input.Vertex = reinterpret_cast<uint8_t*>(helloWorld);
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
    buffer.release();
    saveResult("hello.bmp");
  }
  {
    ImmutableModel sphere = ImmutableModel::CreateSphere(0.5f, 32);
    auto vsSimple = [](int index, VertexShaderParams& param) -> Vector4f {
      const VertexVN& v = param.CastVertex<VertexVN>()[index];
      const CBuffer& cbuffer = param.CastCBuffer<CBuffer>();
      Vector3f& out = param.CastOut<Vector3f>(index);
      out = v.Nor;  //没有model矩阵，直接赋值完事
      return cbuffer.mvp * v.Pos.XYZ1();
    };
    auto psSimple = [](const PixelShaderParams& param, bool& isDiscard) -> Color4f {
      const Vector3f& out = param.CastIn<Vector3f>();
      Vector3f normal = Normalize(out);
      Vector3f lightDir = Normalize(Vector3f(1.0f, 1.0f, 1.0f));
      float cosTheta = Dot(normal, lightDir);
      return Color4f({std::max(0.0f, cosTheta)}, 1.0f);
      return Color4f(1.0f);
    };

    VertexVN v[3];
    PipelineState pso = Renderer::DefaultPSO(vsSimple, psSimple, sizeof(VertexVN), sizeof(Vector3f));
    PipelineInput input;
    input.Vertex = reinterpret_cast<uint8_t*>(v);
    input.CBuffer = reinterpret_cast<uint8_t*>(&mvpBuffer);
    input.ColorBuffer = &cb;
    input.DepthBuffer = &db;
    input.FrameWidth = width;
    input.FrameHeight = height;
    PipelineMemory memory;
    memory.Arena = &buffer;

    Matrix4f viewMat = LookAt(Vector3f(0.0f, 0.0f, 1.25f), Vector3f(0.0f), Vector3f(0.0f, 1.0f, 0.0f));
    Matrix4f projMat = Perspective(Radian(60.0f), (float)width / height, 0.1f, 10.0f);
    mvpBuffer.mvp = projMat * viewMat;

    cb.Fill({0.2f, 0.2f, 0.3f, 1.0f});
    db.Fill(1.0f);

    auto drawCall = [&]() -> void {
      for (size_t i = 0; i < sphere.GetIndexCount(); i += 3) {  //手动input assembly
        for (size_t j = 0; j < 3; j++) {
          v[j].Pos = sphere.GetPositions()[sphere.GetIndices()[i + j]];
          v[j].Nor = sphere.GetNormals()[sphere.GetIndices()[i + j]];
        }
        Renderer::DrawTriangle(input, pso, memory);
        buffer.release();
      }
    };

    drawCall();
    saveResult("light.bmp");

    cb.Fill({0.2f, 0.2f, 0.3f, 1.0f});
    db.Fill(1.0f);
    pso.IsDrawFrame = true;

    viewMat = LookAt(Vector3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 0.0f, 1.0f), Vector3f(0.0f, 1.0f, 0.0f));
    mvpBuffer.mvp = projMat * viewMat;

    drawCall();
    saveResult("cull.bmp");
  }
  return 0;
}