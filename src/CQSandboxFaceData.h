#ifndef CQSandboxFaceData_H
#define CQSandboxFaceData_H

#include <CPoint3D.h>
#include <CVector3D.h>

#include <QColor>

class CGeomFace3D;

class CQGLBuffer;
class CQGLTexture;

namespace CQSandbox {

struct FaceData {
  // data
  CQGLBuffer* buffer { nullptr }; // associated buffer
  int         pos    { 0 };       // buffer data position
  int         len    { 0 };       // buffer data length

  int glMode { -1 }; // gl draw mode

  CVector3D normal;

  // xref
  CGeomFace3D* face { nullptr }; // associated geometry face

  // material
  QColor       color           { Qt::black };
  CQGLTexture* diffuseTexture  { nullptr };
  CQGLTexture* specularTexture { nullptr };
  CQGLTexture* normalTexture   { nullptr };
  CQGLTexture* emissiveTexture { nullptr };
#if 0
  CRGBA        ambient         { 0, 0, 0 };
  CRGBA        diffuse         { 1, 1, 1 };
  CRGBA        specular        { 0, 0, 0 };
  CRGBA        emission        { 0, 0, 0, 0 };
  double       shininess       { 1.0 };
  double       alpha           { 1.0 };
#endif

  // state
#if 0
  bool selected  { false };
  bool visible   { true };
  bool wireframe { false };
  int  lineWidth { -1 };
#endif
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

struct LineData {
  CQGLBuffer* buffer { nullptr }; // associated buffer
  int         ind1   { 0 };
  int         ind2   { 0 };
};

struct PointData {
  CQGLBuffer* buffer { nullptr }; // associated buffer
  int         ind    { 0 };
};

}

#endif
