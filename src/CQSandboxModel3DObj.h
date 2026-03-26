#ifndef CQSandboxModel3DObj_H
#define CQSandboxModel3DObj_H

#include <CQSandboxObject3D.h>

class CGLTexture;
class CQGLBuffer;
class CImportBase;
class CGeomObject3D;

namespace CQSandbox {

class ShaderProgram;

class Model3DObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Model3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "Model"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  bool load(const QString &filename);

  void tick() override;

  void render() override;

  void initShader();

  void updateObjectData();

  void calcTangents();

  void setModelMatrix(uint flags=ModelMatrixFlags::ALL) override;

 protected:
  struct FaceData {
    int         pos             { 0 };
    int         len             { 0 };
    CGLTexture *diffuseTexture  { nullptr };
    CGLTexture *specularTexture { nullptr };
    CGLTexture *normalTexture   { nullptr };
  };

  using FaceDatas = std::vector<FaceData>;

  //FaceDatas faceDatas;

  struct ObjectData {
    CQGLBuffer* buffer { nullptr };
    FaceDatas   faceDatas;
  };

  using ObjectDatas = std::map<CGeomObject3D *, ObjectData *>;
  using GLTextures  = std::map<int, CGLTexture *>;

  //---

  static ShaderProgram* s_program;

  QString vertShaderFile_;
  QString fragShaderFile_;

  QString filename_;

  bool needsUpdate_ { true };

  CImportBase* import_ { nullptr };

  CGLTexture *diffuseTexture_  { nullptr };
  CGLTexture *specularTexture_ { nullptr };
  CGLTexture *normalTexture_   { nullptr };

  CPoint3D    sceneCenter_ { 0 , 0, 0 };
  ObjectDatas objectDatas_;

  bool flipYZ_ { false };

  GLTextures glTextures_;
};

}

#endif
