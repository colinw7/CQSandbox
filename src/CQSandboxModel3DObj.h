#ifndef CQSandboxModel3DObj_H
#define CQSandboxModel3DObj_H

#include <CQSandboxObject3D.h>

#include <CImagePtr.h>

class CQGLTexture;
class CGeomObject3D;
class CGeomTexture;

namespace CQSandbox {

class Texture;
class ShaderProgram;

//---

class Model3DObjMgr : public ObjectMgr3D {
 public:
  Model3DObjMgr() { }

  const char *typeName() const override { return "model"; }

  void initRender(Canvas3D *canvas) override;
  void termRender(Canvas3D *canvas) override;

 private:
  double t_ { 0.0 };
};

//---

class Model3DObj : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  static ShaderProgram* shaderProgram() { return s_shaderData.program; }

  static void initShader(Canvas3D *canvas);

  static void initDraw(Canvas3D *canvas, double t);
  static void termDraw(Canvas3D *canvas);

  //---

  Model3DObj(Canvas3D *canvas);

  //---

  const char *typeName() const override { return "model"; }

  virtual ObjectMgr3D *mgr() override { return s_objectMgr; }

  //---

  bool isAutoScale() const { return autoScale_; }
  void setAutoScale(bool b) { autoScale_ = b; }

  CGeomObject3D *object() const { return object_; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  bool exec(const QString &op, const QStringList &args, QVariant &res) override;

  bool load(const QString &filename);

  void tick() override;

  CBBox3D calcBBox() override { return bbox_; }

  const FaceDatas &getFaceDatas() const override;

  void render() override;

  void calcTangents();

  void setModelMatrix(uint flags=ModelMatrixFlags::ALL) override;

 private:
  void updateObject(CGeomObject3D *object);

  void drawObject(CGeomObject3D *object);

  void updateObjectData();

  //---

  CQGLTexture *getGLTexture(CGeomTexture *texture, bool /*add*/);

  void initGLTexture(Texture *texture);

  CQGLTexture *makeTexture(const CImagePtr &image) const;

  //---

  void calcTangents1(CGeomObject3D *object);

 protected:
  struct TextureBuffer {
    CQGLTexture*   texture       { nullptr };
    ShaderProgram* shaderProgram { nullptr };
    CQGLBuffer*    buffer        { nullptr };
    FaceDataList   faceDataList;
  };

  //---

  struct ShaderData {
    ShaderProgram* program { nullptr };
    QString        vertShaderFile;
    QString        fragShaderFile;
  };

  static ShaderData     s_shaderData;
  static Model3DObjMgr* s_objectMgr;

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
