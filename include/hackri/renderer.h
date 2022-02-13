#ifndef __HACKRI_RENDERER_H__
#define __HACKRI_RENDERER_H__

#include <hackri/mathematics.h>
#include <hackri/buffer.h>
#include <hackri/memory_util.h>
#include <functional>
#include <memory>
#include <memory_resource>

namespace hackri {
struct VertexShaderParams {
  const uint8_t* Vertex;   //顶点数据输入，只读
  uint8_t* Out[3];         //顶点着色器输出，理论上只写
  const uint8_t* CBuffer;  //常量buffer，只读

  template <class T>
  constexpr const T* CastVertex() const noexcept { return reinterpret_cast<const T*>(Vertex); }
  template <class T>
  constexpr T& CastOut(size_t i) const noexcept { return *reinterpret_cast<T*>(Out[i]); }
  template <class T>
  constexpr const T& CastCBuffer() const noexcept { return *reinterpret_cast<const T*>(CBuffer); }
};
struct PixelShaderParams {
  const uint8_t* PixelIn;  //像素着色器输入，只读
  const uint8_t* CBuffer;  //常量buffer，只读

  template <class T>
  constexpr const T& CastIn() const noexcept { return *reinterpret_cast<const T*>(PixelIn); }
  template <class T>
  constexpr const T& CastCBuffer() const noexcept { return *reinterpret_cast<const T*>(CBuffer); }
};
//VS第一个参数是三角形编号，只有[0,1,2]，因为只处理三角形图元
//返回齐次空间下的坐标
using VertexShader = std::function<Vector4f(int, VertexShaderParams&)>;
//PS没啥好说的，很正常的输入输出
using PixelShader = std::function<Color4f(PixelShaderParams&)>;
struct VertexShaderOutLayout {
  size_t Size;  //输出数据大小（字节），必须是sizeof(float)的整数倍
};
enum class DepthComparison {
  Never,
  Less,
  Equal,
  LessEqual,
  Greater,
  NotEqual,
  GreaterEqual,
  Always,
};
enum class CullMode {
  None,
  Back,
  Front,
  BackAndFront  //emm这真的有用吗...
};
enum class FrontFace {
  CCW,  //counter clock wise逆时针
  CW
};
//名字取自DX12的PSO（233
struct PipelineState {
  VertexShader VS;
  PixelShader PS;
  size_t VertexSize;                                  //一个顶点大小（字节）
  VertexShaderOutLayout OutLayout;                    //顶点着色器输出的布局
  size_t CBufferSize;                                 //cbuffer大小
  bool IsDrawFrame = false;                           //是不是线框模式
  DepthComparison DepthFunc = DepthComparison::Less;  //深度比较
  CullMode Cull = CullMode::None;
  FrontFace FrontOrder = FrontFace::CCW;
};
struct PipelineInput {
  uint8_t* Vertex;   //顶点数据输入
  uint8_t* CBuffer;  //常量buffer
  uint32_t FrameWidth;
  uint32_t FrameHeight;
  Buffer2d<Color4f>* ColorBuffer;  //最终颜色
  Buffer2d<float>* DepthBuffer;    //深度缓冲
};
struct PipelineContext {
  uint8_t* VsOut;     //需要长度是PSO里面的OutLayout.Size * 3
  uint8_t* PsIn;      //需要长度是PSO里面的OutLayout.Size
  Vector4f* ClipPos;  //需要长度是Vector4f * 8
  uint8_t* ClipOut;   //需要长度是PSO里面的OutLayout.Size * 8
};
struct PipelineMemory {
  std::pmr::monotonic_buffer_resource* Arena;  //管线执行时内存分配器

  template <class T>
  T* Allocate(size_t count, size_t align = alignof(std::max_align_t)) {
    return reinterpret_cast<T*>(Arena->allocate(sizeof(T) * count, align));
  }
  template <class T>
  Span<T> AllocToSpan(size_t count, size_t align = alignof(std::max_align_t)) {
    return Span<T>(Allocate<T>(count, align), count);
  }
};

class Renderer {
 public:
  static void DrawLine(
      uint32_t x1, uint32_t y1,
      uint32_t x2, uint32_t y2,
      const Color4f& color, Buffer2d<Color4f>& buffer);
  static void DrawLine(
      uint32_t x1, uint32_t y1,
      uint32_t x2, uint32_t y2,
      const Color3b& color, Buffer2d<Color3b>& buffer);

  //如果顶点包含{Pos,Normal,Tangant}三种属性
  //那内存中排布只能是
  //{P,N,T}{P,N,T}...
  //不可以是PP...NN...TT...
  //其他排列只能通过input assembly转换成这个排列再输入
  //
  //如果顶点着色器会输出{Normal,Tangant,Color}
  //在内存中排布就是
  //{N,T,C}{N,T,C}{N,T,C}
  //最后像素着色器使用插值后的结果是
  //{N,T,C}
  //（为啥是这样排列？自己规定的，好写）
  //
  //NDC和OpenGL看齐，大小[-1,1]^3
  //
  //目前深度测试和OpenGL默认看齐，z越小越靠近near
  //
  //所以NDC理论上是左手系，摄像机应该看向z+，向上朝y+，右侧指向x+
  //
  //只有float插值运算，其他类型插值原地爆炸，所以VertexShaderParams::Out[3]实际上是3个float数组
  //
  //输入的顶点、VS输出的Out、CBuffer都是byte指针，由外部保证不越界（就是PSO里填的和真实使用的大小要对应
  static void DrawTriangle(
      const PipelineInput& input,
      const PipelineState& pso,
      PipelineMemory& memory);
};
}  // namespace hackri

#endif