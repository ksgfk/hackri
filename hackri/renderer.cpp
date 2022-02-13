#include <hackri/renderer.h>

#include <cassert>

using namespace hackri;
//################
//# Bresenham画线 #
//################
template <class T>
static void Bresenham(uint32_t x1, uint32_t y1,
                      uint32_t x2, uint32_t y2,
                      const T& color, Buffer2d<T>& buffer) {
  uint32_t x, y;
  if (x1 == x2 && y1 == y2) {
    buffer(x1, y1) = color;
    return;
  } else if (x1 == x2) {
    int inc = (y1 <= y2) ? 1 : -1;
    for (y = y1; y != y2; y += inc) buffer(x1, y) = color;
    buffer(x2, y2) = color;
  } else if (y1 == y2) {
    int inc = (x1 <= x2) ? 1 : -1;
    for (x = x1; x != x2; x += inc) buffer(x, y1) = color;
    buffer(x2, y2) = color;
  } else {
    int dx = (x1 < x2) ? x2 - x1 : x1 - x2;
    int dy = (y1 < y2) ? y2 - y1 : y1 - y2;
    int rem = 0;
    if (dx >= dy) {
      if (x2 < x1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
      for (x = x1, y = y1; x <= x2; x++) {
        buffer(x, y) = color;
        rem += dy;
        if (rem >= dx) {
          rem -= dx;
          y += (y2 >= y1) ? 1 : -1;
          buffer(x, y) = color;
        }
      }
      buffer(x2, y2) = color;
    } else {
      if (y2 < y1) x = x1, y = y1, x1 = x2, y1 = y2, x2 = x, y2 = y;
      for (x = x1, y = y1; y <= y2; y++) {
        buffer(x, y) = color;
        rem += dx;
        if (rem >= dy) {
          rem -= dy;
          x += (x2 >= x1) ? 1 : -1;
          buffer(x, y) = color;
        }
      }
      buffer(x2, y2) = color;
    }
  }
}
void Renderer::DrawLine(
    uint32_t x1, uint32_t y1,
    uint32_t x2, uint32_t y2,
    const Color4f& color, Buffer2d<Color4f>& buffer) {
  Bresenham(x1, y1, x2, y2, color, buffer);
}
void Renderer::DrawLine(
    uint32_t x1, uint32_t y1,
    uint32_t x2, uint32_t y2,
    const Color3b& color, Buffer2d<Color3b>& buffer) {
  Bresenham(x1, y1, x2, y2, color, buffer);
}
//###########
//# 光栅管线 #
//###########
static int IsInClipSpace(const Vector4f& v) noexcept {
  return std::abs(v.X()) <= v.W() && std::abs(v.Y()) <= v.W() && std::abs(v.Z()) <= v.W();
}
static Vector3f ViewportTransform(uint32_t width, uint32_t height, const Vector3f& ndc) noexcept {
  float x = (ndc.X() + 1) * 0.5f * (float)width;   // [-1, 1] -> [0, w]
  float y = (ndc.Y() + 1) * 0.5f * (float)height;  // [-1, 1] -> [0, h]
  float z = (ndc.Z() + 1) * 0.5f;                  // [-1, 1] -> [0, 1]
  return Vector3f(x, y, z);
}
static Array<uint32_t, 4> FindBoundingBox(const Span<Vector2f>& p, uint32_t width, uint32_t height) noexcept {
  Vector2f minConer = SelectMin(SelectMin(p[0], p[1]), p[2]);
  Vector2f maxConer = SelectMax(SelectMax(p[0], p[1]), p[2]);
  Array<uint32_t, 4> bbox;
  bbox[0] = (uint32_t)std::max((int)std::floor(minConer.X()), 0);
  bbox[1] = (uint32_t)std::max((int)std::floor(minConer.Y()), 0);
  bbox[2] = (uint32_t)std::min((int)std::ceil(maxConer.X()), (int)width - 1);
  bbox[3] = (uint32_t)std::min((int)std::ceil(maxConer.Y()), (int)height - 1);
  return bbox;
}
static bool IsSameSide(const Vector2f& pa, const Vector2f& pb, const Vector2f& a, const Vector2f& b) noexcept {
  float e1 = Cross(b - a, pa - a);
  float e2 = Cross(b - a, pb - a);
  return (e1 * e2) >= 0;
}
static bool IsInTriangle(const Vector2f& p, const Vector2f* tri) noexcept {
  return IsSameSide(p, tri[0], tri[1], tri[2]) &&
         IsSameSide(p, tri[1], tri[0], tri[2]) &&
         IsSameSide(p, tri[2], tri[0], tri[1]);
}
static Vector3f GetBaryCoord(const Vector2f& p, const Span<Vector2f>& tri) noexcept {
  const Vector2f& a = tri[0];
  const Vector2f& b = tri[1];
  const Vector2f& c = tri[2];
  Vector2f ab = b - a;
  Vector2f ac = c - a;
  Vector2f ap = p - a;
  float factor = 1 / Cross(ab, ac);
  float s = Cross(ap, ac) * factor;
  float t = Cross(ab, ap) * factor;
  return {1 - s - t, s, t};
}
static bool IsInTriangle(const Vector3f& bary) {
  return bary.X() >= -EPSILON &&
         bary.Y() >= -EPSILON &&
         bary.Z() >= -EPSILON;
}
static float InterpolateDepth(const Vector3f& bary, const Span<float>& depth) {
  float depth0 = depth[0] * bary.X();
  float depth1 = depth[1] * bary.Y();
  float depth2 = depth[2] * bary.Z();
  return depth0 + depth1 + depth2;
}
constexpr static void LerpProperties(
    float delta,
    const float* u, const float* v,
    float* w, size_t len) noexcept {
  for (size_t i = 0; i < len; i++) {
    w[i] = Lerp(delta, u[i], v[i]);
  }
}
enum class ClipPlane : size_t {
  PositiveW,  //把W轴裁一点点，这里裁0.00005，避免除0结果nan
  PositiveX,
  NegativeX,
  PositiveY,
  NegativeY,
  PositiveZ,
  NegativeZ,
  CLIP_SIZE
};
constexpr float W_CLIP = float(1e-5);
constexpr static bool IsInsidePlane(const Vector4f& coord, ClipPlane plane) {
  switch (plane) {
    case ClipPlane::PositiveW:
      return coord.W() >= W_CLIP;
    case ClipPlane::PositiveX:
      return coord.X() <= +coord.W();
    case ClipPlane::NegativeX:
      return coord.X() >= -coord.W();
    case ClipPlane::PositiveY:
      return coord.Y() <= +coord.W();
    case ClipPlane::NegativeY:
      return coord.Y() >= -coord.W();
    case ClipPlane::PositiveZ:
      return coord.Z() <= +coord.W();
    case ClipPlane::NegativeZ:
      return coord.Z() >= -coord.W();
    default:
      return false;
  }
}
constexpr static float IntersectRatio(const Vector4f& prev, const Vector4f& curr, ClipPlane plane) {
  switch (plane) {
    case ClipPlane::PositiveW:
      return (prev.W() - W_CLIP) / (prev.W() - curr.W());
    case ClipPlane::PositiveX:
      return (prev.W() - prev.X()) / ((prev.W() - prev.X()) - (curr.W() - curr.X()));
    case ClipPlane::NegativeX:
      return (prev.W() + prev.X()) / ((prev.W() + prev.X()) - (curr.W() + curr.X()));
    case ClipPlane::PositiveY:
      return (prev.W() - prev.Y()) / ((prev.W() - prev.Y()) - (curr.W() - curr.Y()));
    case ClipPlane::NegativeY:
      return (prev.W() + prev.Y()) / ((prev.W() + prev.Y()) - (curr.W() + curr.Y()));
    case ClipPlane::PositiveZ:
      return (prev.W() - prev.Z()) / ((prev.W() - prev.Z()) - (curr.W() - curr.Z()));
    case ClipPlane::NegativeZ:
      return (prev.W() + prev.Z()) / ((prev.W() + prev.Z()) - (curr.W() + curr.Z()));
    default:
      return 0;
  }
}
static void SutherlandHodgemanAlgo(
    ClipPlane plane,
    std::pmr::vector<Vector4f>& outPos, std::pmr::vector<Vector4f>& inPos,
    std::pmr::vector<Span<uint8_t>>& outOut, std::pmr::vector<Span<uint8_t>> inOut,
    size_t length, size_t floatCount,
    PipelineMemory& memory) {
  size_t inCount = inPos.size();
  for (size_t i = 0; i < inCount; i++) {
    size_t prev = (i - 1 + inCount) % inCount;
    size_t curr = i;
    bool isPrevInside = IsInsidePlane(inPos[prev], plane);
    bool isCurrInside = IsInsidePlane(inPos[curr], plane);
    if (isPrevInside != isCurrInside) {
      float ratio = IntersectRatio(inPos[prev], inPos[curr], plane);
      Span<uint8_t> newPoint = memory.AllocToSpan<uint8_t>(length);
      LerpProperties(
          ratio,
          (float*)inOut[prev].GetPointer(), (float*)inOut[curr].GetPointer(),
          (float*)newPoint.GetPointer(), floatCount);
      outPos.emplace_back(Lerp(ratio, inPos[prev], inPos[curr]));
      outOut.emplace_back(newPoint);
    }
    if (isCurrInside) {
      outPos.emplace_back(inPos[curr]);
      outOut.emplace_back(inOut[curr]);
    }
  }
}
static std::tuple<size_t, std::pmr::vector<Vector4f>, std::pmr::vector<Span<uint8_t>>> SutherlandHodgeman(
    const Vector4f& clipA, const Vector4f& clipB, const Vector4f& clipC,
    const Span<uint8_t>& outA, const Span<uint8_t>& outB, const Span<uint8_t>& outC, size_t length, size_t floatCount,
    PipelineMemory& memory) {
  std::pmr::vector<Vector4f> outputPos(memory.Arena);
  outputPos.reserve(8);
  outputPos.assign({clipA, clipB, clipC});
  std::pmr::vector<Span<uint8_t>> outputOut(memory.Arena);
  outputOut.reserve(8);
  outputOut.assign({outA, outB, outC});
  if (IsInClipSpace(clipA) && IsInClipSpace(clipB) && IsInClipSpace(clipC)) {
    return std::make_tuple(3, outputPos, outputOut);
  }
  std::pmr::vector<Vector4f> inputPos(memory.Arena);
  inputPos.reserve(8);
  std::pmr::vector<Span<uint8_t>> inputOut(memory.Arena);
  inputOut.reserve(8);
  for (size_t i = 0; i < 7; i++) {
    if (i % 2 == 0) {
      inputPos.clear();
      inputOut.clear();
      SutherlandHodgemanAlgo(
          (ClipPlane)i,
          inputPos, outputPos,
          inputOut, outputOut, length, floatCount,
          memory);
    } else {
      outputPos.clear();
      outputOut.clear();
      SutherlandHodgemanAlgo(
          (ClipPlane)i,
          outputPos, inputPos,
          outputOut, inputOut, length, floatCount,
          memory);
    }
  }
  return std::make_tuple(outputPos.size(), std::move(outputPos), std::move(outputOut));
}
constexpr static bool DepthTest(float depth, float target, DepthComparison func) noexcept {
  switch (func) {
    case hackri::DepthComparison::Never:
      return false;
    case hackri::DepthComparison::Less:
      return depth < target;
    case hackri::DepthComparison::Equal:
      return std::abs(depth - target) <= float(1e-5);
    case hackri::DepthComparison::LessEqual:
      return depth <= target;
    case hackri::DepthComparison::Greater:
      return depth > target;
    case hackri::DepthComparison::NotEqual:
      return std::abs(depth - target) > float(1e-5);
    case hackri::DepthComparison::GreaterEqual:
      return depth >= target;
    case hackri::DepthComparison::Always:
      return true;
    default:
      return false;
  }
}
static void DrawInterpolateLine(
    const PipelineInput& input, const PipelineState& pso,
    const Array<uint32_t, 2>& a, const Array<uint32_t, 2>& b,
    float depthA, float depthB,
    Span<float> outA, Span<float> outB, Span<float>& psIn, size_t len) {
  PixelShaderParams psParam{psIn.Cast<uint8_t>().GetPointer(), input.CBuffer};
  auto& cb = *input.ColorBuffer;
  auto depthTestAndWrite = [&](float delta, uint32_t x, uint32_t y) -> void {
    if (input.DepthBuffer != nullptr) {
      float depth = Lerp(delta, depthA, depthB);
      if (!DepthTest(depth, (*input.DepthBuffer)(x, y), pso.DepthFunc)) {
        return;
      }
      (*input.DepthBuffer)(x, y) = depth;
    }
    LerpProperties(delta, outA.GetPointer(), outB.GetPointer(), psIn.GetPointer(), len);
    cb(x, y) = pso.PS(psParam);
  };
  uint32_t x1 = a.X(), y1 = a.Y(), x2 = b.X(), y2 = b.Y();
  if (x1 == x2 && y1 == y2) {
    depthTestAndWrite(0.0f, x1, y1);
  } else if (x1 == x2) {
    int inc = (y1 <= y2) ? 1 : -1;
    int dy = (y1 < y2) ? y2 - y1 : y1 - y2;
    for (uint32_t y = y1; y != y2; y += inc) {
      float delta = std::abs(((float)y - y1)) / dy;
      depthTestAndWrite(delta, x1, y);
    }
    depthTestAndWrite(1.0f, x2, y2);
  } else if (y1 == y2) {
    int inc = (x1 <= x2) ? 1 : -1;
    int dx = (x1 < x2) ? x2 - x1 : x1 - x2;
    for (uint32_t x = x1; x != x2; x += inc) {
      float delta = std::abs(((float)x - x1)) / dx;
      depthTestAndWrite(delta, x, y1);
    }
    depthTestAndWrite(1.0f, x2, y2);
  } else {
    int rem = 0;
    int dx = (x1 < x2) ? x2 - x1 : x1 - x2;
    int dy = (y1 < y2) ? y2 - y1 : y1 - y2;
    if (dx >= dy) {
      if (x2 < x1) {
        std::swap(x1, x2);
        std::swap(y1, y2);
        std::swap(outA, outB);
        std::swap(depthA, depthB);
      }
      for (uint32_t x = x1, y = y1; x <= x2; x++) {
        float delta = std::abs((float)x - x1) / dx;
        depthTestAndWrite(delta, x, y);
        rem += dy;
        if (rem >= dx) {
          rem -= dx;
          y += (y2 >= y1) ? 1 : -1;
          if (y >= input.FrameHeight - 1) y = input.FrameHeight - 1;
          float delta = std::abs((float)y - y1) / dy;
          depthTestAndWrite(delta, x, y);
        }
      }
      depthTestAndWrite(1.0f, x2, y2);
    } else {
      if (y2 < y1) {
        std::swap(x1, x2);
        std::swap(y1, y2);
        std::swap(outA, outB);
        std::swap(depthA, depthB);
      }
      for (uint32_t x = x1, y = y1; y <= y2; y++) {
        float delta = std::abs((float)y - y1) / dy;
        depthTestAndWrite(delta, x, y);
        rem += dx;
        if (rem >= dx) {
          rem -= dy;
          x += (x2 >= x1) ? 1 : -1;
          if (x >= input.FrameWidth - 1) x = input.FrameWidth - 1;
          float delta = std::abs((float)x - x1) / dx;
          depthTestAndWrite(delta, x, y);
        }
      }
      depthTestAndWrite(1.0f, x2, y2);
    }
  }
}
static bool IsCull(const Span<Vector3f>& ndc, CullMode mode, FrontFace face) noexcept {
  float area = ndc[0].X() * ndc[1].Y() - ndc[0].Y() * ndc[1].X() +
               ndc[1].X() * ndc[2].Y() - ndc[1].Y() * ndc[2].X() +
               ndc[2].X() * ndc[0].Y() - ndc[2].Y() * ndc[0].X();
  switch (mode) {
    case hackri::CullMode::None:
      return false;
    case hackri::CullMode::Back:
      return face == FrontFace::CCW ? area <= 0 : area > 0;
    case hackri::CullMode::Front:
      return face == FrontFace::CCW ? area > 0 : area <= 0;
    case hackri::CullMode::BackAndFront:
      return true;
    default:
      return false;
  }
}
void Renderer::DrawTriangle(
    const PipelineInput& input,
    const PipelineState& pso,
    PipelineMemory& memory) {
  const size_t vsOutSize = pso.OutLayout.Size;             //顶点着色器输出大小
  const size_t vsOutFloatCnt = vsOutSize / sizeof(float);  //需要插值数量
  assert((vsOutSize % sizeof(float)) == 0);                //所有输出都必须可以插值
  //初始化内存
  Span<Vector4f> clipPos = memory.AllocToSpan<Vector4f>(3);
  Span<uint8_t> vsOutA = memory.AllocToSpan<uint8_t>(vsOutSize);
  Span<uint8_t> vsOutB = memory.AllocToSpan<uint8_t>(vsOutSize);
  Span<uint8_t> vsOutC = memory.AllocToSpan<uint8_t>(vsOutSize);
  Span<uint8_t> psIn = memory.AllocToSpan<uint8_t>(vsOutSize);
  Span<Vector3f> ndc = memory.AllocToSpan<Vector3f>(3);
  Span<Vector2f> scrPos = memory.AllocToSpan<Vector2f>(3);
  Span<float> depthZ = memory.AllocToSpan<float>(3);
  //运行VS，计算顶点在clip space的坐标
  for (int i = 0; i < 3; i++) {
    VertexShaderParams vsParam{input.Vertex,
                               {vsOutA.GetPointer(), vsOutB.GetPointer(), vsOutC.GetPointer()},
                               input.CBuffer};
    clipPos[i] = pso.VS(i, vsParam);
  }
  //齐次空间裁剪
  const auto [vertexCount, outPos, outOut] = SutherlandHodgeman(
      clipPos[0], clipPos[1], clipPos[2],
      vsOutA, vsOutB, vsOutC, vsOutSize, vsOutFloatCnt,
      memory);
  for (int i = 0; i < (int)vertexCount - 2; i++) {
    const Vector4f& clipA = outPos[0];
    const Vector4f& clipB = outPos[i + 1];
    const Vector4f& clipC = outPos[i + 2];
    const Span<float> outA = outOut[0].Cast<float>();
    const Span<float> outB = outOut[i + 1].Cast<float>();
    const Span<float> outC = outOut[i + 2].Cast<float>();
    Span<float> pixelInput = psIn.Cast<float>();
    //透视除法，将顶点转化到规范化设备坐标(NDC)
    Vector3f invW(1.0f / clipA.W(), 1.0f / clipB.W(), 1.0f / clipC.W());
    ndc[0] = clipA.XYZ() * invW[0];
    ndc[1] = clipB.XYZ() * invW[1];
    ndc[2] = clipC.XYZ() * invW[2];
    //面剔除
    if (IsCull(ndc, pso.Cull, pso.FrontOrder)) {
      continue;
    }
    //视口变换，转化到屏幕空间坐标
    for (int i = 0; i < 3; i++) {
      Vector3f sp = ViewportTransform(input.FrameWidth, input.FrameHeight, ndc[i]);
      scrPos[i] = sp.XY();
      depthZ[i] = sp.Z();
    }
    if (pso.IsDrawFrame) {
      constexpr auto toInt = [](const Vector2f& v, uint32_t w, uint32_t h) -> Array<uint32_t, 2> {
        return Array<uint32_t, 2>(
            (uint32_t)std::floor(std::clamp(v.X(), 0.0f, (float)w - 1)),
            (uint32_t)std::floor(std::clamp(v.Y(), 0.0f, (float)h - 1)));
      };
      auto la = toInt(scrPos[0], input.FrameWidth, input.FrameHeight);
      auto lb = toInt(scrPos[1], input.FrameWidth, input.FrameHeight);
      auto lc = toInt(scrPos[2], input.FrameWidth, input.FrameHeight);
      DrawInterpolateLine(input, pso, la, lb, depthZ[0], depthZ[1], outA, outB, pixelInput, vsOutFloatCnt);
      DrawInterpolateLine(input, pso, la, lc, depthZ[0], depthZ[2], outA, outC, pixelInput, vsOutFloatCnt);
      DrawInterpolateLine(input, pso, lb, lc, depthZ[1], depthZ[2], outB, outC, pixelInput, vsOutFloatCnt);
    } else {
      //根据屏幕空间坐标计算包围盒
      Array<uint32_t, 4> bbox = FindBoundingBox(scrPos, input.FrameWidth, input.FrameHeight);
      for (uint32_t x = bbox[0]; x <= bbox[2]; x++) {
        for (uint32_t y = bbox[1]; y <= bbox[3]; y++) {
          Vector2f point((float)x + 0.5f, (float)y + 0.5f);
          //计算重心坐标用于插值
          Vector3f bary = GetBaryCoord(point, scrPos);
          //如果重心坐标出现小于0，说明屏幕坐标位于三角形外部
          if (!IsInTriangle(bary)) {
            continue;
          }
          //插值深度
          float depth = InterpolateDepth(bary, depthZ);
          //深度测试
          if (input.DepthBuffer != nullptr) {
            if (!DepthTest(depth, (*input.DepthBuffer)(x, y), pso.DepthFunc)) {
              continue;
            }
            //更新深度值
            (*input.DepthBuffer)(x, y) = depth;
          }
          //插值顶点属性。透视矫正，使用inv w作为权重
          Vector3f weight = invW * bary;
          float normalize = 1.0f / (weight[0] + weight[1] + weight[2]);
          for (size_t i = 0; i < vsOutFloatCnt; i++) {
            float sum = outA[i] * weight.X() + outB[i] * weight.Y() + outC[i] * weight.Z();
            pixelInput[i] = sum * normalize;
          }
          //使用插值后的结果计算像素颜色
          PixelShaderParams psParam{psIn.GetPointer(), input.CBuffer};
          Color4f color = pso.PS(psParam);
          (*input.ColorBuffer)(x, y) = color;
        }
      }
    }
  }
}