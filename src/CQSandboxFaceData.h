#ifndef CQSandboxFaceData_H
#define CQSandboxFaceData_H

class CGLTexture;

namespace CQSandbox {

struct FaceData {
  int         pos             { 0 };
  int         len             { 0 };
  CGLTexture* diffuseTexture  { nullptr };
  CGLTexture* specularTexture { nullptr };
  CGLTexture* normalTexture   { nullptr };
  CGLTexture* emissiveTexture { nullptr };
};

}

#endif
