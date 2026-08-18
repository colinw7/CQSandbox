#ifndef CQSandboxFaceData_H
#define CQSandboxFaceData_H

#include <QColor>

class CGeomFace3D;
class CGLTexture;

namespace CQSandbox {

struct FaceData {
  CGeomFace3D* face            { nullptr };
  int          pos             { 0 };
  int          len             { 0 };
  QColor       color           { Qt::black };
  CGLTexture*  diffuseTexture  { nullptr };
  CGLTexture*  specularTexture { nullptr };
  CGLTexture*  normalTexture   { nullptr };
  CGLTexture*  emissiveTexture { nullptr };
};

}

#endif
