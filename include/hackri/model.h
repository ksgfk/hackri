#ifndef __HACKRI_MODEL_H__
#define __HACKRI_MODEL_H__

#include <hackri/mathematics.h>
#include <vector>
#include <string>
#include <istream>
#include <memory>

namespace hackri {
//顶点从0开始数
class ImmutableModel {
 public:
  ImmutableModel() noexcept;
  ImmutableModel(std::vector<Vector3f>&& pos,
                 std::vector<Vector3f>&& nor,
                 std::vector<Vector2f>&& tex,
                 std::vector<size_t>&& ind);
  ImmutableModel(std::vector<Vector3f>&& pos,
                 std::vector<Vector3f>&& nor,
                 std::vector<Vector2f>&& tex,
                 std::vector<Vector4f>&& tan,
                 std::vector<size_t>&& ind);
  ImmutableModel(const ImmutableModel&) = delete;
  ImmutableModel(ImmutableModel&&) noexcept;
  ImmutableModel& operator=(ImmutableModel&&) noexcept;
  ~ImmutableModel();

  const std::vector<Vector3f>& GetPositions() const { return _positions; }
  const std::vector<Vector3f>& GetNormals() const { return _normals; }
  const std::vector<Vector2f>& GetTexCoords() const { return _texcoords; }
  const std::vector<Vector4f>& GetTangents() const { return _tangent; }
  const std::vector<size_t>& GetIndices() const { return _indices; }

  bool HasNormal() const;
  bool HasTexCoord() const;
  bool HasTangent() const;

  size_t GetVertexCount() const;
  size_t GetIndexCount() const;
  size_t GetTriangleCount() const;

  static ImmutableModel CreateSphere(float radius, int numberSlices);
  static ImmutableModel CreateCube(float halfExtend);
  static ImmutableModel CreateQuad(float halfExtend);
  static ImmutableModel CreateCylinder(float halfExtend, float radius, int numberSlices);
  static ImmutableModel CreateCylinder(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount);
  static ImmutableModel CreateGrid(float width, float depth, uint32_t m, uint32_t n);

 private:
  std::vector<Vector3f> _positions;
  std::vector<Vector3f> _normals;
  std::vector<Vector2f> _texcoords;
  std::vector<Vector4f> _tangent;
  std::vector<size_t> _indices;
};

class WavefrontObjReader {
 public:
  //所有index都是大于0的整数
  //如果出现0说明是不存在的索引（相当于nullptr）
  //就是数组从1开始数（obj文件就是这样做的
  struct Face {
    Array<size_t, 3> Position;
    Array<size_t, 3> Normal;
    Array<size_t, 3> TexCoord;
  };
  struct ModelObject {
    std::string Name;
    std::vector<size_t> Faces;
    std::vector<std::string> Materials;
  };

  WavefrontObjReader(std::unique_ptr<std::istream>&& stream);
  WavefrontObjReader(const std::string& path);
  ~WavefrontObjReader();
  WavefrontObjReader(const WavefrontObjReader&) = delete;
  WavefrontObjReader(WavefrontObjReader&&) noexcept;

  void Parse();

  const std::vector<Vector3f>& GetPosition() const { return _position; }
  const std::vector<Vector3f>& GetNormal() const { return _normal; }
  const std::vector<Vector2f>& GetTexCoord() const { return _texCoord; }
  const std::vector<Face>& GetFace() const { return _face; }
  const std::vector<ModelObject>& GetObject() const { return _object; }
  const std::vector<std::string>& GetMtlPath() const { return _mtlPath; }
  bool HasError() const;
  const std::string& GetError() const { return _error; }
  size_t GetVertexCount() const;

  static std::optional<Vector2f> ParseVec2(std::string_view str);
  static std::optional<Vector3f> ParseVec3(std::string_view str);
  static std::optional<WavefrontObjReader::Face> ParseFace(std::string_view str);

  ImmutableModel ToModel() const;

 private:
  std::vector<Vector3f> _position;
  std::vector<Vector3f> _normal;
  std::vector<Vector2f> _texCoord;
  std::vector<Face> _face;
  std::vector<ModelObject> _object;
  std::vector<std::string> _mtlPath;
  std::unique_ptr<std::istream> _stream;
  std::string _error;
};
}  // namespace hackri

#endif