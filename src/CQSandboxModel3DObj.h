#ifndef CQSandboxModel3DObj_H
#define CQSandboxModel3DObj_H

#include <CQSandboxObject3D.h>
#include <CQSandboxFaceData.h>

#include <CImagePtr.h>

class CQGLBuffer;
class CQGLTexture;
class CGeomObject3D;
class CGeomTexture;

namespace CQSandbox {

class Texture;
class ShaderProgram;

class Model3DObj : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  static ShaderProgram* shaderProgram() { return s_program; }

  //---

  Model3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "model"; }

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

  void initDraw(double t);

  void drawObject(CGeomObject3D *object, double t);

  void updateObjectData();

  //---

  CQGLTexture *getGLTexture(CGeomTexture *texture, bool /*add*/);

  void initGLTexture(Texture *texture);

  CQGLTexture *makeTexture(const CImagePtr &image) const;

  //---

  void calcTangents1(CGeomObject3D *object);

 protected:
  using FaceDatas = std::vector<FaceData>;

  //---

  struct TextureBuffer {
    CQGLTexture*   texture       { nullptr };
    ShaderProgram* shaderProgram { nullptr };
    CQGLBuffer*    buffer        { nullptr };
    FaceDataList   faceDataList;
  };

  //---

  static ShaderProgram* s_program;

  QString vertShaderFile_;
  QString fragShaderFile_;

  QString filename_;

  bool needsUpdate_ { true };

  CGeomObject3D* object_ { nullptr };

  CQGLTexture* diffuseTexture_  { nullptr };
  CQGLTexture* specularTexture_ { nullptr };
  CQGLTexture* normalTexture_   { nullptr };
  CQGLTexture* emissiveTexture_ { nullptr };

  CPoint3D sceneCenter_ { 0 , 0, 0 };

  bool flipYZ_      { false };
  bool autoScale_   { false };
  bool transformed_ { false };
};

}

#endif
