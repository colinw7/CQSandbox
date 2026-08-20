#ifndef CQSandboxFaceData_H
#define CQSandboxFaceData_H

#include <QColor>

class CGeomFace3D;

class CQGLTexture;

namespace CQSandbox {

struct FaceData {
  CGeomFace3D* face            { nullptr };
  int          pos             { 0 };
  int          len             { 0 };
  QColor       color           { Qt::black };
  CQGLTexture* diffuseTexture  { nullptr };
  CQGLTexture* specularTexture { nullptr };
  CQGLTexture* normalTexture   { nullptr };
  CQGLTexture* emissiveTexture { nullptr };
};

struct FaceDataList {
  void clear() { faceDatas.clear(); pos = 0; }

  void add(const FaceDataList &faceDataList) {
    for (const auto &faceData : faceDataList.faceDatas)
      faceDatas.push_back(faceData);
  }

  int                   pos { 0 };
  std::vector<FaceData> faceDatas;
};

}

#endif
