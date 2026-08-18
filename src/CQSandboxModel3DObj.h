#ifndef CQSandboxModel3DObj_H
#define CQSandboxModel3DObj_H

#include <CQSandboxObject3D.h>
#include <CQSandboxFaceData.h>

class CGLTexture;
class CQGLBuffer;
class CGeomObject3D;

namespace CQSandbox {

class ShaderProgram;

class Model3DObj : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  static ShaderProgram* shaderProgram() { return s_program; }

  //---

  Model3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "Model"; }

  bool isAutoScale() const { return autoScale_; }
  void setAutoScale(bool b) { autoScale_ = b; }

  CGeomObject3D *object() const { return object_; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  bool exec(const QString &op, const QStringList &args, QVariant &res) override;

  bool load(const QString &filename);

  void tick() override;

  void render() override;

  void initShader();

  void calcTangents();

  void setModelMatrix(uint flags=ModelMatrixFlags::ALL) override;

 private:
  void updateObject(CGeomObject3D *object);

  void drawObject(CGeomObject3D *object, double t);

  void updateObjectData();

  void calcTangents1(CGeomObject3D *object);

 protected:
  using FaceDatas  = std::vector<FaceData>;
  using GLTextures = std::map<int, CGLTexture *>;

  //---

  static ShaderProgram* s_program;

  QString vertShaderFile_;
  QString fragShaderFile_;

  QString filename_;

  bool needsUpdate_ { true };

  CGeomObject3D* object_ { nullptr };

  CGLTexture* diffuseTexture_  { nullptr };
  CGLTexture* specularTexture_ { nullptr };
  CGLTexture* normalTexture_   { nullptr };
  CGLTexture* emissiveTexture_ { nullptr };

  CPoint3D sceneCenter_ { 0 , 0, 0 };

  bool flipYZ_      { false };
  bool autoScale_   { false };
  bool transformed_ { false };

  GLTextures glTextures_;
};

}

#endif
