#ifndef CQSandboxShape3DObj_H
#define CQSandboxShape3DObj_H

#include <CQSandboxObject3D.h>
#include <CQSandboxShape3DData.h>
#include <CQSandboxMaterial3D.h>

#include <CGLVector2D.h>
#include <CGLColor.h>

class CQGLTexture;
class CShape3D;

namespace CQSandbox {

class ShaderProgram;

//---

class Shape3DObjMgr : public ObjectMgr3D {
 public:
  Shape3DObjMgr() { }

  const char *typeName() const override { return "shape"; }

  void initRender(Canvas3D *canvas) override;
  void termRender(Canvas3D *canvas) override;
};

//---

class Shape3DObj : public Object3D {
  Q_OBJECT

  Q_PROPERTY(ShapeType shapeType READ shapeType)

  Q_PROPERTY(QString diffuseTexture  READ diffuseTextureFile  WRITE setDiffuseTextureFile)
  Q_PROPERTY(QString normalTexture   READ normalTextureFile   WRITE setNormalTextureFile)
  Q_PROPERTY(QString specularTexture READ specularTextureFile WRITE setSpecularTextureFile)
  Q_PROPERTY(QString emissionTexture READ emissionTextureFile WRITE setEmissionTextureFile)

  Q_ENUMS(ShapeType)

 public:
  enum class ShapeType {
    NONE,
    CONE,
    CUBE,
    CYLINDER,
    SPHERE
  };

  //---

  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  static ShaderProgram* shaderProgram() { return s_program; }

  static void initShader(Canvas3D *canvas);

  static void initDraw(Canvas3D *canvas);
  static void termDraw(Canvas3D *canvas);

  //---

  Shape3DObj(Canvas3D *canvas);

  //---

  virtual ObjectMgr3D *mgr() override { return s_objectMgr; }

  //---

  const char *typeName() const override { return "shape"; }

  //---

  const ShapeType &shapeType() const { return shapeType_; }

  const Shape3DData &shapeData() const { return shapeData_; }

  //---

  QString diffuseTextureFile() const;
  void setDiffuseTextureFile(const QString &filename);

  void setDiffuseTexture(CQGLTexture *texture);

  QString normalTextureFile() const;
  void setNormalTextureFile(const QString &filename);

  void setNormalTexture(CQGLTexture *texture);

  QString specularTextureFile() const;
  void setSpecularTextureFile(const QString &filename);

  void setSpecularTexture(CQGLTexture *texture);

  QString emissionTextureFile() const;
  void setEmissionTextureFile(const QString &filename);

  void setEmissionTexture(CQGLTexture *texture);

  //---

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void updateGL();

  bool intersect(const CVector3D &p1, const CVector3D &p2,
                 CPoint3D &pi1, CPoint3D &pi2) const override;

  CBBox3D calcBBox() override;

  void calcNormals();

  const FaceDatas &getFaceDatas() const override;

  void render() override;

  void addCube(double sx, double sy, double sz);

  //---

  void applyMatrix(const CMatrix3DH &m) override;

 protected:
  using Colors = std::vector<CGLColor>;

  static ShaderProgram* s_program;
  static Shape3DObjMgr* s_objectMgr;

  ShapeType   shapeType_ { ShapeType::NONE };
  Shape3DData shapeData_;

  Colors colors_;
  bool   wireframe_ { false };

  Material3D* material_ { nullptr };
};

}

#endif
