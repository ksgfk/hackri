#include <hackri/model.h>

#include <fstream>
#include <string_view>
#include <algorithm>
#include <optional>
#include <utility>
#include <stdexcept>
#include <unordered_map>

using namespace hackri;

ImmutableModel::ImmutableModel() noexcept = default;

ImmutableModel::ImmutableModel(
    std::vector<Vector3f>&& pos,
    std::vector<Vector3f>&& nor,
    std::vector<Vector2f>&& tex,
    std::vector<size_t>&& ind) {
  _positions = std::move(pos);
  _normals = std::move(nor);
  _texcoords = std::move(tex);
  _indices = std::move(ind);
  if (_indices.size() % 3 != 0) {
    throw std::invalid_argument("indices must an integer multiple of 3");
  }
}

ImmutableModel::ImmutableModel(
    std::vector<Vector3f>&& pos,
    std::vector<Vector3f>&& nor,
    std::vector<Vector2f>&& tex,
    std::vector<Vector4f>&& tan,
    std::vector<size_t>&& ind) {
  _positions = std::move(pos);
  _normals = std::move(nor);
  _texcoords = std::move(tex);
  _tangent = std::move(tan);
  _indices = std::move(ind);
  if (_indices.size() % 3 != 0) {
    throw std::invalid_argument("indices must an integer multiple of 3");
  }
}

ImmutableModel::ImmutableModel(ImmutableModel&& other) noexcept {
  _positions = std::move(other._positions);
  _normals = std::move(other._normals);
  _texcoords = std::move(other._texcoords);
  _tangent = std::move(other._tangent);
  _indices = std::move(other._indices);
}

ImmutableModel& ImmutableModel::operator=(ImmutableModel&& other) noexcept {
  _positions = std::move(other._positions);
  _normals = std::move(other._normals);
  _texcoords = std::move(other._texcoords);
  _tangent = std::move(other._tangent);
  _indices = std::move(other._indices);
  return *this;
}

ImmutableModel::~ImmutableModel() = default;

bool ImmutableModel::HasNormal() const { return _normals.size() > 0; }

bool ImmutableModel::HasTexCoord() const { return _texcoords.size() > 0; }

bool ImmutableModel::HasTangent() const { return _tangent.size() > 0; }

size_t ImmutableModel::GetVertexCount() const { return _positions.size(); }

size_t ImmutableModel::GetIndexCount() const { return _indices.size(); }

size_t ImmutableModel::GetTriangleCount() const { return _indices.size() / 3; }

ImmutableModel ImmutableModel::CreateSphere(float radius, int numberSlices) {
  const Vector3f axisX = {1.0f, 0.0f, 0.0f};

  uint32_t numberParallels = numberSlices / 2;
  uint32_t numberVertices = (numberParallels + 1) * (numberSlices + 1);
  uint32_t numberIndices = numberParallels * numberSlices * 6;

  float angleStep = (2.0f * PI) / ((float)numberSlices);

  std::vector<Vector3f> vertices(numberVertices, Vector3f{});
  std::vector<Vector3f> normals(numberVertices, Vector3f{});
  std::vector<Vector2f> texCoords(numberVertices, Vector2f{});
  std::vector<Vector4f> tangents(numberVertices, Vector4f{});

  for (uint32_t i = 0; i < numberParallels + 1; i++) {
    for (uint32_t j = 0; j < (uint32_t)(numberSlices + 1); j++) {
      uint32_t vertexIndex = (i * (numberSlices + 1) + j);
      uint32_t normalIndex = (i * (numberSlices + 1) + j);
      uint32_t texCoordsIndex = (i * (numberSlices + 1) + j);
      uint32_t tangentIndex = (i * (numberSlices + 1) + j);

      float px = radius * std::sin(angleStep * (float)i) * std::sin(angleStep * (float)j);
      float py = radius * std::cos(angleStep * (float)i);
      float pz = radius * std::sin(angleStep * (float)i) * std::cos(angleStep * (float)j);
      vertices[vertexIndex] = {px, py, pz};

      float nx = vertices[vertexIndex].X() / radius;
      float ny = vertices[vertexIndex].Y() / radius;
      float nz = vertices[vertexIndex].Z() / radius;
      normals[normalIndex] = {nx, ny, nz};

      float tx = (float)j / (float)numberSlices;
      float ty = 1.0f - (float)i / (float)numberParallels;
      texCoords[texCoordsIndex] = {tx, ty};

      auto mat = AxisAngle({0, 1.0f, 0}, 360.0f * texCoords[texCoordsIndex].X());
      auto t = mat * axisX.XYZ1();
      tangents[tangentIndex] = t.XYZ1();
    }
  }

  uint32_t indexIndices = 0;
  std::vector<size_t> indices(numberIndices, size_t{});
  for (uint32_t i = 0; i < numberParallels; i++) {
    for (uint32_t j = 0; j < (uint32_t)(numberSlices); j++) {
      indices[indexIndices++] = i * ((size_t)numberSlices + 1) + j;
      indices[indexIndices++] = ((size_t)i + 1) * ((size_t)numberSlices + 1) + j;
      indices[indexIndices++] = ((size_t)i + 1) * ((size_t)numberSlices + 1) + ((size_t)j + 1);

      indices[indexIndices++] = i * ((size_t)numberSlices + 1) + j;
      indices[indexIndices++] = ((size_t)i + 1) * ((size_t)numberSlices + 1) + ((size_t)j + 1);
      indices[indexIndices++] = (size_t)i * ((size_t)numberSlices + 1) + ((size_t)j + 1);
    }
  }

  return ImmutableModel(std::move(vertices),
                        std::move(normals),
                        std::move(texCoords),
                        std::move(tangents),
                        std::move(indices));
}

ImmutableModel ImmutableModel::CreateCube(float halfExtend) {
  constexpr const float cubeVertices[] =
      {-1.0f, -1.0f, -1.0f, +1.0f, -1.0f, -1.0f, +1.0f, +1.0f, +1.0f, -1.0f, +1.0f, +1.0f, +1.0f, -1.0f, -1.0f, +1.0f,
       -1.0f, +1.0f, -1.0f, +1.0f, -1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, -1.0f, +1.0f,
       -1.0f, -1.0f, -1.0f, +1.0f, -1.0f, +1.0f, -1.0f, +1.0f, +1.0f, +1.0f, -1.0f, +1.0f, +1.0f, -1.0f, -1.0f, +1.0f,
       -1.0f, -1.0f, +1.0f, +1.0f, -1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, -1.0f, +1.0f, +1.0f,
       -1.0f, -1.0f, -1.0f, +1.0f, -1.0f, -1.0f, +1.0f, +1.0f, -1.0f, +1.0f, +1.0f, +1.0f, -1.0f, +1.0f, -1.0f, +1.0f,
       +1.0f, -1.0f, -1.0f, +1.0f, +1.0f, -1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, +1.0f, -1.0f, +1.0f};
  constexpr const float cubeNormals[] =
      {0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f,
       0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f,
       0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f,
       0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f,
       -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
       +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f};
  constexpr const float cubeTexCoords[] =
      {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
       0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
       1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
       0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
       0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
       1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
  constexpr const float cubeTangents[] =
      {+1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f,
       +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f,
       -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
       +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f,
       0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f, 0.0f, 0.0f, +1.0f,
       0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, -1.0f};
  constexpr const size_t cubeIndices[] =
      {0, 2, 1, 0, 3, 2,
       4, 5, 6, 4, 6, 7,
       8, 9, 10, 8, 10, 11,
       12, 15, 14, 12, 14, 13,
       16, 17, 18, 16, 18, 19,
       20, 23, 22, 20, 22, 21};

  constexpr const uint32_t numberVertices = 24;
  constexpr const uint32_t numberIndices = 36;

  std::vector<Vector3f> vertices(numberVertices, Vector3f{});
  std::vector<Vector3f> normals(numberVertices, Vector3f{});
  std::vector<Vector2f> texCoords(numberVertices, Vector2f{});
  std::vector<Vector4f> tangents(numberVertices, Vector4f{});
  for (uint32_t i = 0; i < numberVertices; i++) {
    vertices[i] = {cubeVertices[i * 4 + 0] * halfExtend,
                   cubeVertices[i * 4 + 1] * halfExtend,
                   cubeVertices[i * 4 + 2] * halfExtend};
    normals[i] = {cubeNormals[i * 3 + 0],
                  cubeNormals[i * 3 + 1],
                  cubeNormals[i * 3 + 2]};
    texCoords[i] = {cubeTexCoords[i * 2 + 0],
                    cubeTexCoords[i * 2 + 1]};
    tangents[i] = {cubeTangents[i * 2 + 0],
                   cubeTangents[i * 2 + 1],
                   cubeTangents[i * 2 + 2],
                   1.0f};
  }
  std::vector<size_t> indices(cubeIndices, cubeIndices + numberIndices);
  return ImmutableModel(std::move(vertices),
                        std::move(normals),
                        std::move(texCoords),
                        std::move(tangents),
                        std::move(indices));
}

ImmutableModel ImmutableModel::CreateQuad(float halfExtend) {
  constexpr const float quadVertices[] =
      {-1.0f, -1.0f, 0.0f, +1.0f,
       +1.0f, -1.0f, 0.0f, +1.0f,
       -1.0f, +1.0f, 0.0f, +1.0f,
       +1.0f, +1.0f, 0.0f, +1.0f};
  constexpr const float quadNormal[] =
      {0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f,
       0.0f, 0.0f, 1.0f};
  constexpr const float quadTex[] =
      {0.0f, 0.0f,
       1.0f, 0.0f,
       0.0f, 1.0f,
       1.0f, 1.0f};
  constexpr const float quadTan[] =
      {1.0f, 0.0f, 0.0f,
       1.0f, 0.0f, 0.0f,
       1.0f, 0.0f, 0.0f,
       1.0f, 0.0f, 0.0f};
  constexpr const size_t quadIndices[] =
      {0, 1, 2,
       1, 3, 2};
  constexpr const uint32_t numberVertices = 4;
  constexpr const uint32_t numberIndices = 6;
  std::vector<Vector3f> vertices(numberVertices, Vector3f{});
  std::vector<Vector3f> normals(numberVertices, Vector3f{});
  std::vector<Vector2f> texCoords(numberVertices, Vector2f{});
  std::vector<Vector4f> tangents(numberVertices, Vector4f{});
  for (uint32_t i = 0; i < numberVertices; i++) {
    vertices[i] = {quadVertices[i * 4 + 0] * halfExtend,
                   quadVertices[i * 4 + 1] * halfExtend,
                   quadVertices[i * 4 + 2]};
    normals[i] = {quadNormal[i * 3 + 0],
                  quadNormal[i * 3 + 1],
                  quadNormal[i * 3 + 2]};
    texCoords[i] = {quadTex[i * 2 + 0],
                    quadTex[i * 2 + 1]};
    tangents[i] = {quadTan[i * 3 + 0],
                   quadTan[i * 3 + 1],
                   quadTan[i * 3 + 2],
                   1.0f};
  }
  std::vector<size_t> indices(quadIndices, quadIndices + numberIndices);
  return ImmutableModel(std::move(vertices),
                        std::move(normals),
                        std::move(texCoords),
                        std::move(tangents),
                        std::move(indices));
}

ImmutableModel ImmutableModel::CreateCylinder(float halfExtend, float radius, int numberSlices) {
  int numberVertices = (numberSlices + 2) * 2 + (numberSlices + 1) * 2;
  int numberIndices = numberSlices * 3 * 2 + numberSlices * 6;
  float angleStep = (2.0f * PI) / ((float)numberSlices);
  std::vector<Vector3f> positions(numberVertices, Vector3f{});
  std::vector<Vector3f> normals(numberVertices, Vector3f{});
  std::vector<Vector2f> texCoords(numberVertices, Vector2f{});
  std::vector<Vector4f> tangents(numberVertices, Vector4f{});
  std::vector<size_t> indices(numberIndices, 0);

  size_t vertexCounter = 0;
  // Center bottom
  positions[vertexCounter][0] = 0.0f;
  positions[vertexCounter][1] = -halfExtend;
  positions[vertexCounter][2] = 0.0f;

  normals[vertexCounter][0] = 0.0f;
  normals[vertexCounter][1] = -1.0f;
  normals[vertexCounter][2] = 0.0f;

  tangents[vertexCounter][0] = 0.0f;
  tangents[vertexCounter][1] = 0.0f;
  tangents[vertexCounter][2] = 1.0f;
  tangents[vertexCounter][3] = 1.0f;

  texCoords[vertexCounter][0] = 0.0f;
  texCoords[vertexCounter][1] = 0.0f;

  vertexCounter++;
  // Bottom
  for (int i = 0; i < numberSlices + 1; i++) {
    float currentAngle = angleStep * (float)i;

    positions[vertexCounter][0] = std::cos(currentAngle) * radius;
    positions[vertexCounter][1] = -halfExtend;
    positions[vertexCounter][2] = -std::sin(currentAngle) * radius;

    normals[vertexCounter][0] = 0.0f;
    normals[vertexCounter][1] = -1.0f;
    normals[vertexCounter][2] = 0.0f;

    tangents[vertexCounter][0] = std::sin(currentAngle);
    tangents[vertexCounter][1] = 0.0f;
    tangents[vertexCounter][2] = std::cos(currentAngle);
    tangents[vertexCounter][3] = 1.0f;

    texCoords[vertexCounter][0] = 0.0f;
    texCoords[vertexCounter][1] = 0.0f;

    vertexCounter++;
  }
  // Center top
  positions[vertexCounter][0] = 0.0f;
  positions[vertexCounter][1] = halfExtend;
  positions[vertexCounter][2] = 0.0f;

  normals[vertexCounter][0] = 0.0f;
  normals[vertexCounter][1] = 1.0f;
  normals[vertexCounter][2] = 0.0f;

  tangents[vertexCounter][0] = 0.0f;
  tangents[vertexCounter][1] = 0.0f;
  tangents[vertexCounter][2] = -1.0f;
  tangents[vertexCounter][3] = 1.0f;

  texCoords[vertexCounter][0] = 1.0f;
  texCoords[vertexCounter][1] = 1.0f;

  vertexCounter++;
  // Top
  for (int i = 0; i < numberSlices + 1; i++) {
    float currentAngle = angleStep * (float)i;

    positions[vertexCounter][0] = std::cos(currentAngle) * radius;
    positions[vertexCounter][1] = halfExtend;
    positions[vertexCounter][2] = -std::sin(currentAngle) * radius;

    normals[vertexCounter][0] = 0.0f;
    normals[vertexCounter][1] = 1.0f;
    normals[vertexCounter][2] = 0.0f;

    tangents[vertexCounter][0] = -std::sin(currentAngle);
    tangents[vertexCounter][1] = 0.0f;
    tangents[vertexCounter][2] = -std::cos(currentAngle);
    tangents[vertexCounter][3] = 1.0f;

    texCoords[vertexCounter][0] = 1.0f;
    texCoords[vertexCounter][1] = 1.0f;

    vertexCounter++;
  }
  for (int i = 0; i < numberSlices + 1; i++) {
    float currentAngle = angleStep * (float)i;
    float sign = -1.0f;
    for (int j = 0; j < 2; j++) {
      positions[vertexCounter][0] = std::cos(currentAngle) * radius;
      positions[vertexCounter][1] = halfExtend * sign;
      positions[vertexCounter][2] = -std::sin(currentAngle) * radius;

      normals[vertexCounter][0] = std::cos(currentAngle);
      normals[vertexCounter][1] = 0.0f;
      normals[vertexCounter][2] = -std::sin(currentAngle);

      tangents[vertexCounter][0] = -std::sin(currentAngle);
      tangents[vertexCounter][1] = 0.0f;
      tangents[vertexCounter][2] = -std::cos(currentAngle);
      tangents[vertexCounter][3] = 1.0f;

      texCoords[vertexCounter][0] = (float)i / (float)numberSlices;
      texCoords[vertexCounter][1] = (sign + 1.0f) / 2.0f;

      vertexCounter++;

      sign = 1.0f;
    }
  }
  // index
  // Bottom
  size_t centerIndex = 0;
  size_t indexCounter = 1;
  size_t indexIndices = 0;

  for (int i = 0; i < numberSlices; i++) {
    indices[indexIndices++] = centerIndex;
    indices[indexIndices++] = indexCounter + 1;
    indices[indexIndices++] = indexCounter;

    indexCounter++;
  }
  indexCounter++;

  // Top
  centerIndex = indexCounter;
  indexCounter++;

  for (int i = 0; i < numberSlices; i++) {
    indices[indexIndices++] = centerIndex;
    indices[indexIndices++] = indexCounter;
    indices[indexIndices++] = indexCounter + 1;

    indexCounter++;
  }
  indexCounter++;

  // Sides
  for (int i = 0; i < numberSlices; i++) {
    indices[indexIndices++] = indexCounter;
    indices[indexIndices++] = indexCounter + 2;
    indices[indexIndices++] = indexCounter + 1;

    indices[indexIndices++] = indexCounter + 2;
    indices[indexIndices++] = indexCounter + 3;
    indices[indexIndices++] = indexCounter + 1;

    indexCounter += 2;
  }

  return ImmutableModel(std::move(positions),
                        std::move(normals),
                        std::move(texCoords),
                        std::move(tangents),
                        std::move(indices));
}

ImmutableModel ImmutableModel::CreateCylinder(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount) {
  float stackHeight = height / stackCount;
  float radiusStep = (topRadius - bottomRadius) / stackCount;
  uint32_t ringCount = stackCount + 1;

  std::vector<Vector3f> positions;
  std::vector<Vector3f> normals;
  std::vector<Vector2f> texCoords;
  std::vector<Vector4f> tangents;
  std::vector<size_t> indices;

  for (uint32_t i = 0; i < ringCount; ++i) {
    float y = -0.5f * height + i * stackHeight;
    float r = bottomRadius + i * radiusStep;
    float dTheta = 2.0f * PI / sliceCount;
    for (uint32_t j = 0; j <= sliceCount; ++j) {
      float c = std::cos(j * dTheta);
      float s = std::sin(j * dTheta);
      Vector3f position(r * c, y, r * s);

      Vector2f texCoord{};
      texCoord.X() = (float)j / sliceCount;
      texCoord.Y() = 1.0f - (float)i / stackCount;

      Vector4f tangent(-s, 0.0f, c, 1.0f);

      float dr = bottomRadius - topRadius;
      Vector3f bitangent(dr * c, -height, dr * s);

      Vector3f T = tangent.XYZ();
      Vector3f B = bitangent;
      Vector3f N = Normalize(Cross(T, B));
      Vector3f normal = N;

      positions.emplace_back(position);
      texCoords.emplace_back(texCoord);
      tangents.emplace_back(tangent);
      normals.emplace_back(normal);
    }
  }
  size_t ringVertexCount = size_t(sliceCount) + 1;
  for (size_t i = 0; i < stackCount; ++i) {
    for (size_t j = 0; j < sliceCount; ++j) {
      indices.push_back(i * ringVertexCount + j);
      indices.push_back((i + 1) * ringVertexCount + j);
      indices.push_back((i + 1) * ringVertexCount + j + 1);

      indices.push_back(i * ringVertexCount + j);
      indices.push_back((i + 1) * ringVertexCount + j + 1);
      indices.push_back(i * ringVertexCount + j + 1);
    }
  }
  {  //TOP
    size_t baseIndex = positions.size();
    float y = 0.5f * height;
    float dTheta = 2.0f * PI / sliceCount;
    for (size_t i = 0; i <= sliceCount; ++i) {
      float x = topRadius * std::cos(i * dTheta);
      float z = topRadius * std::sin(i * dTheta);
      float u = x / height + 0.5f;
      float v = z / height + 0.5f;

      positions.emplace_back(Vector3f{x, y, z});
      texCoords.emplace_back(Vector2f{u, v});
      tangents.emplace_back(Vector4f{1.0f, 0.0f, 0.0f, 1.0f});
      normals.emplace_back(Vector3f{0.0f, 1.0f, 0.0f});
    }
    positions.emplace_back(Vector3f{0.0f, y, 0.0f});
    texCoords.emplace_back(Vector2f{0.5f, 0.5f});
    tangents.emplace_back(Vector4f{1.0f, 0.0f, 0.0f, 1.0f});
    normals.emplace_back(Vector3f{0.0f, 1.0f, 0.0f});
    size_t centerIndex = positions.size() - 1;
    for (size_t i = 0; i < sliceCount; ++i) {
      indices.push_back(centerIndex);
      indices.push_back(baseIndex + i + 1);
      indices.push_back(baseIndex + i);
    }
  }
  {  //bottom
    size_t baseIndex = positions.size();
    float y = -0.5f * height;
    float dTheta = 2.0f * PI / sliceCount;
    for (size_t i = 0; i <= sliceCount; ++i) {
      float x = bottomRadius * cosf(i * dTheta);
      float z = bottomRadius * sinf(i * dTheta);
      float u = x / height + 0.5f;
      float v = z / height + 0.5f;

      positions.emplace_back(Vector3f{x, y, z});
      texCoords.emplace_back(Vector2f{u, v});
      tangents.emplace_back(Vector4f{1.0f, 0.0f, 0.0f, 1.0f});
      normals.emplace_back(Vector3f{0.0f, -1.0f, 0.0f});
    }
    positions.emplace_back(Vector3f{0.0f, y, 0.0f});
    texCoords.emplace_back(Vector2f{0.5f, 0.5f});
    tangents.emplace_back(Vector4f{1.0f, 0.0f, 0.0f, 1.0f});
    normals.emplace_back(Vector3f{0.0f, -1.0f, 0.0f});
    size_t centerIndex = positions.size() - 1;
    for (size_t i = 0; i < sliceCount; ++i) {
      indices.push_back(centerIndex);
      indices.push_back(baseIndex + i);
      indices.push_back(baseIndex + i + 1);
    }
  }
  positions.shrink_to_fit();
  normals.shrink_to_fit();
  texCoords.shrink_to_fit();
  tangents.shrink_to_fit();
  indices.shrink_to_fit();
  return ImmutableModel(std::move(positions),
                        std::move(normals),
                        std::move(texCoords),
                        std::move(tangents),
                        std::move(indices));
}

ImmutableModel ImmutableModel::CreateGrid(float width, float depth, uint32_t m, uint32_t n) {
  size_t vertexCount = size_t(m) * size_t(n);
  size_t faceCount = (size_t(m) - 1) * (size_t(n) - 1) * 2;
  std::vector<Vector3f> positions(vertexCount);
  std::vector<Vector3f> normals(vertexCount);
  std::vector<Vector2f> texCoords(vertexCount);
  std::vector<Vector4f> tangents(vertexCount);
  std::vector<size_t> indices(faceCount * 3);
  float halfWidth = 0.5f * width;
  float halfDepth = 0.5f * depth;
  float dx = width / (n - 1);
  float dz = depth / (m - 1);
  float du = 1.0f / (n - 1);
  float dv = 1.0f / (m - 1);
  for (size_t i = 0; i < m; ++i) {
    float z = halfDepth - i * dz;
    for (size_t j = 0; j < n; ++j) {
      float x = -halfWidth + j * dx;
      positions[i * n + j] = Vector3f(x, 0.0f, z);
      normals[i * n + j] = Vector3f(0.0f, 1.0f, 0.0f);
      tangents[i * n + j] = Vector4f(1.0f, 0.0f, 0.0f, 1.0f);
      texCoords[i * n + j].X() = j * du;
      texCoords[i * n + j].Y() = i * dv;
    }
  }
  size_t k = 0;
  for (size_t i = 0; i < size_t(m) - 1; ++i) {
    for (size_t j = 0; j < size_t(n) - 1; ++j) {
      indices[k] = i * n + j;
      indices[k + 1] = i * n + j + 1;
      indices[k + 2] = (i + 1) * n + j;
      indices[k + 3] = (i + 1) * n + j;
      indices[k + 4] = i * n + j + 1;
      indices[k + 5] = (i + 1) * n + j + 1;
      k += 6;
    }
  }
  return ImmutableModel(std::move(positions),
                        std::move(normals),
                        std::move(texCoords),
                        std::move(tangents),
                        std::move(indices));
}

WavefrontObjReader::WavefrontObjReader(std::unique_ptr<std::istream>&& stream) {
  _stream = std::move(stream);
  //索引从1开始数
  _position.emplace_back(Vector3f{});
  _normal.emplace_back(Vector3f{});
  _texCoord.emplace_back(Vector2f{});
}

WavefrontObjReader::WavefrontObjReader(const std::string& path)
    : WavefrontObjReader(std::make_unique<std::ifstream>(path, std::ifstream::in)) {}

WavefrontObjReader::~WavefrontObjReader() = default;

WavefrontObjReader::WavefrontObjReader(WavefrontObjReader&& o) noexcept {
  _position = std::move(o._position);
  _normal = std::move(o._normal);
  _texCoord = std::move(o._texCoord);
  _face = std::move(o._face);
  _object = std::move(o._object);
  _stream = std::move(o._stream);
}

//##############
//# 读取Obj文件 #
//##############
static bool IsWhiteSpace(const std::string& str) noexcept {
  for (char c : str) {
    if (c != ' ') {
      return false;
    }
  }
  return true;
}
static bool Split3(std::string_view str, Array<size_t, 2>& splitIdx, char split) noexcept {
  size_t splitCnt = 0;
  for (size_t i = 0; i < str.size(); i++) {
    if (str[i] == split) {
      splitIdx[splitCnt++] = i;
      if (splitCnt == 2) {
        break;
      }
    }
  }
  return splitCnt == 2;
}
std::optional<Vector2f> WavefrontObjReader::ParseVec2(std::string_view str) {
  size_t idx = str.find(' ');
  if (idx == std::string_view::npos) {
    return std::nullopt;
  }
  try {
    Vector2f result;
    result[0] = std::stof(std::string(str, 0, idx));
    result[1] = std::stof(std::string(str.substr(idx + 1)));
    return std::make_optional(result);
  } catch (std::invalid_argument&) {
    return std::nullopt;
  }
}
std::optional<Vector3f> WavefrontObjReader::ParseVec3(std::string_view str) {
  Array<size_t, 2> splitIdx;
  if (!Split3(str, splitIdx, ' ')) {
    return std::nullopt;
  }
  try {
    Vector3f result;
    result[0] = std::stof(std::string(str, 0, splitIdx[0]));
    result[1] = std::stof(std::string(str, splitIdx[0] + 1, splitIdx[1] - splitIdx[0] - 1));
    result[2] = std::stof(std::string(str.substr(splitIdx[1] + 1)));
    return std::make_optional<Vector3f>(result);
  } catch (std::invalid_argument&) {
    return std::nullopt;
  }
}
std::optional<WavefrontObjReader::Face> WavefrontObjReader::ParseFace(std::string_view str) {
  //<v,vt,vn>
  auto mode2 = [](std::string_view fstr) -> Array<size_t, 3> {
    Array<size_t, 2> idx;
    Split3(fstr, idx, '/');
    Array<size_t, 3> result;
    if (idx[1] - idx[0] == 1) {  //f v1//vn1 v2//vn2 v3//vn3
      result[0] = static_cast<size_t>(std::stoul(std::string(fstr, 0, idx[0])));
      result[1] = static_cast<size_t>(0);
      result[2] = static_cast<size_t>(std::stoul(std::string(fstr.substr(idx[1] + 1))));
    } else {  //f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
      result[0] = static_cast<size_t>(std::stoul(std::string(fstr, 0, idx[0])));
      result[1] = static_cast<size_t>(std::stoul(std::string(fstr, idx[0] + 1, idx[1] - idx[0] - 1)));
      result[2] = static_cast<size_t>(std::stoul(std::string(fstr.substr(idx[1] + 1))));
    }
    return result;
  };
  auto mode1 = [](std::string_view fstr) -> Array<size_t, 3> {  //f v1/vt1 v2/vt2 v3/vt3
    size_t idx = fstr.find('/');
    Array<size_t, 3> result;
    result[0] = static_cast<size_t>(std::stoul(std::string(fstr, 0, idx)));
    result[1] = static_cast<size_t>(std::stoul(std::string(fstr.substr(idx + 1))));
    result[2] = static_cast<size_t>(0);
    return result;
  };
  auto mode0 = [](std::string_view fstr) -> Array<size_t, 3> {  //f v1 v2 v3
    Array<size_t, 3> result;
    result[0] = static_cast<size_t>(std::stoul(std::string(fstr)));
    result[1] = static_cast<size_t>(0);
    result[2] = static_cast<size_t>(0);
    return result;
  };

  Array<size_t, 2> splitIdx;
  if (!Split3(str, splitIdx, ' ')) {
    return std::nullopt;
  }
  std::string_view a = str.substr(0, splitIdx[0]);
  std::string_view b = str.substr(splitIdx[0] + 1, splitIdx[1] - splitIdx[0] - 1);
  std::string_view c = str.substr(splitIdx[1] + 1, str.size() - splitIdx[1] - 1);
  try {
    size_t mode = 0;
    for (char t : a) {
      if (t == '/') {
        mode++;
      }
    }
    //<v,vt,vn>
    Array<size_t, 3> f1;
    Array<size_t, 3> f2;
    Array<size_t, 3> f3;
    if (mode == 2) {
      f1 = mode2(a);
      f2 = mode2(b);
      f3 = mode2(c);
    } else if (mode == 1) {
      f1 = mode1(a);
      f2 = mode1(b);
      f3 = mode1(c);
    } else if (mode == 0) {
      f1 = mode0(a);
      f2 = mode0(b);
      f3 = mode0(c);
    } else {
      return std::nullopt;
    }
    WavefrontObjReader::Face face{
        {f1[0], f2[0], f3[0]},
        {f1[2], f2[2], f3[2]},
        {f1[1], f2[1], f3[1]}};
    return face;
  } catch (std::invalid_argument&) {
    return std::nullopt;
  }
}
void WavefrontObjReader::Parse() {
  if (_stream == nullptr || !_stream->good() || _stream->eof()) {
    _error += "can't read stream";
    return;
  }
  size_t lineNum = 0;
  std::string line;
  while (!_stream->eof()) {
    std::getline(*_stream, line);
    lineNum++;
    if (line.size() == 0 || IsWhiteSpace(line)) {
      continue;
    }
    if (line.compare(0, 1, "#") == 0) {  //commit
      continue;
    } else if (line.compare(0, 2, "o ") == 0 || line.compare(0, 2, "g ") == 0) {  //object and group
      std::string name = line.substr(2);
      if (std::find_if(_object.begin(), _object.end(),
                       [&](const auto& i) { return i.Name == name; }) == _object.end()) {
        _object.emplace_back(ModelObject{std::move(name), {}});
      } else {
        _error += "duplicate object names\n";
      }
    } else if (line.compare(0, 2, "v ") == 0) {  //position
      std::string_view v = std::string_view(line).substr(2);
      auto result = ParseVec3(v);
      if (result) {
        _position.emplace_back(*result);
      } else {
        _error += "can't read position. line: " + std::to_string(lineNum) + "\n";
      }
    } else if (line.compare(0, 2, "f ") == 0) {  //face
      std::string_view f = std::string_view(line).substr(2);
      auto result = ParseFace(f);
      if (result) {
        _face.emplace_back(*result);
        if (_object.size() > 0) {
          _object.rbegin()->Faces.emplace_back(_face.size() - 1);
        }
      } else {
        _error += "can't read face. line: " + std::to_string(lineNum) + "\n";
      }
    } else if (line.compare(0, 3, "vt ") == 0) {  //texcoord
      std::string_view vt = std::string_view(line).substr(3);
      auto result = ParseVec2(vt);
      if (result) {
        _texCoord.emplace_back(*result);
      } else {
        _error += "can't read texcoord. line: " + std::to_string(lineNum) + "\n";
      }
    } else if (line.compare(0, 3, "vn ") == 0) {  //normal
      std::string_view vn = std::string_view(line).substr(3);
      auto result = ParseVec3(vn);
      if (result) {
        _normal.emplace_back(*result);
      } else {
        _error += "can't read normal. line: " + std::to_string(lineNum) + "\n";
      }
    } else if (line.compare(0, 7, "mtllib ") == 0) {  //引用外部材质文件路径名
      std::string_view mtllib = std::string_view(line).substr(7);
      _mtlPath.emplace_back(std::string(mtllib));
    } else if (line.compare(0, 7, "usemtl ") == 0) {  //挂在object上的材质
      std::string_view usemtl = std::string_view(line).substr(7);
      if (_object.size() > 0) {
        _object.rbegin()->Materials.emplace_back(std::string(usemtl));
      }
    } else if (line.compare(0, 2, "s ") == 0) {  //平滑组
      continue;
    } else {
      _error += "unknown cmd: " + line;
    }
  }
}

bool WavefrontObjReader::HasError() const { return _error.size() > 0; }

size_t WavefrontObjReader::GetVertexCount() const {
  //索引从1开始数
  return _position.size() - 1;
}

struct _VI {
  size_t p;
  size_t n;
  size_t t;
  bool operator<(const _VI& v) const {
    if (p < v.p) return true;
    if (p > v.p) return false;
    if (n < v.n) return true;
    if (n > v.n) return false;
    if (t < v.t) return true;
    if (t > v.t) return false;
    return false;
  }
  size_t operator()(const _VI& v) const {
    return std::hash<size_t>()(v.p) ^
           std::hash<size_t>()(v.n) ^
           std::hash<size_t>()(v.t);
  }
  bool operator==(const _VI& r) const {
    return p == r.p && n == r.n && t == r.t;
  }
};
ImmutableModel WavefrontObjReader::ToModel() const {
  std::vector<Vector3f> position;
  std::vector<Vector3f> normal;
  std::vector<Vector2f> texcoord;
  std::vector<Vector4f> tan;
  std::vector<size_t> indices;
  std::unordered_map<_VI, size_t, _VI> uni;
  size_t count = 0;
  for (const auto& f : GetFace()) {
    for (size_t i = 0; i < 3; i++) {
      _VI vid{f.Position[i], f.Normal[i], f.TexCoord[i]};
      auto [iter, isIn] = uni.try_emplace(vid, count);
      if (isIn) {
        const auto& i = iter->first;
        Vector3f pos = GetPosition()[i.p];
        position.emplace_back(pos);
        if (i.n > 0) {
          Vector3f nor = GetNormal()[i.n];
          normal.emplace_back(nor);
        }
        if (i.t > 0) {
          Vector2f tex = GetTexCoord()[i.t];
          texcoord.emplace_back(tex);
        }
        count++;
      }
      indices.emplace_back(iter->second);
    }
  }
  if (normal.size() > 0 && texcoord.size() > 0) {
    //http://foundationsofgameenginedev.com/FGED2-sample.pdf
    //第9页
    std::vector<Vector3f> tangent(position.size(), Vector3f{});
    std::vector<Vector3f> biTan(position.size(), Vector3f{});
    tan.resize(position.size());
    //计算每个三角形的切线和副切线，叠加到三个顶点上
    for (size_t i = 0; i < indices.size(); i += 3) {
      auto i0 = i + 0;
      auto i1 = i + 1;
      auto i2 = i + 2;
      auto p0 = position[indices[i0]];
      auto p1 = position[indices[i1]];
      auto p2 = position[indices[i2]];
      auto w0 = texcoord[indices[i0]];
      auto w1 = texcoord[indices[i1]];
      auto w2 = texcoord[indices[i2]];

      auto e1 = p1 - p0;
      auto e2 = p2 - p0;
      auto x1 = w1.X() - w0.X();
      auto x2 = w2.X() - w0.X();
      auto y1 = w1.Y() - w0.Y();
      auto y2 = w2.Y() - w0.Y();

      float r = 1.0f / (x1 * y2 - x2 * y1);
      auto t = (e1 * Vector3f(y2) - e2 * Vector3f(y1)) * Vector3f(r);
      auto b = (e2 * Vector3f(x1) - e1 * Vector3f(x2)) * Vector3f(r);

      tangent[indices[i0]] += t;
      tangent[indices[i1]] += t;
      tangent[indices[i2]] += t;
      biTan[indices[i0]] += b;
      biTan[indices[i1]] += b;
      biTan[indices[i2]] += b;
    }
    //正交化所有切线并计算手性
    for (size_t i = 0; i < position.size(); i++) {
      const auto& t = tangent[i];
      const auto& b = biTan[i];
      const auto& n = normal[i];
      //应该是叫Gram-Schmidt process？
      auto xyz = Normalize(Reject(t, n));
      auto w = (Dot(Cross(t, b), n) > 0.0f) ? 1.0f : -1.0f;
      tan[i] = {xyz.X(), xyz.Y(), xyz.Z(), w};
    }
  }

  position.shrink_to_fit();
  normal.shrink_to_fit();
  texcoord.shrink_to_fit();

  return ImmutableModel(
      std::move(position),
      std::move(normal),
      std::move(texcoord),
      std::move(tan),
      std::move(indices));
}