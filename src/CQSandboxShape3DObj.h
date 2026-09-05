#ifndef CQSandboxShape3DObj_H
#define CQSandboxShape3DObj_H

#include <CQSandboxObject3D.h>
#include <CQSandboxShape3DData.h>

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
  Q_PROPERTY(QString   texture   READ textureFile WRITE setTextureFile)

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

  const char *typeName() const override { return "shape"; }

  virtual ObjectMgr3D *mgr() override { return s_objectMgr; }

  //---

  const ShapeType &shapeType() const { return shapeType_; }

  const Shape3DData &shapeData() const { return shapeData_; }

  //---

  const CGLColor &color() const { return color_; }
  void setColor(const CGLColor &c) { color_ = c; }

  const QString &textureFile() const { return textureFile_; }
  void setTextureFile(const QString &filename);

  void setTexture(CQGLTexture *texture) { diffuseTexture_ = texture; }

  void setNormalTexture(const QString &filename);
  void setNormalTexture(CQGLTexture *texture) { normalTexture_ = texture; }

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

 protected:
  using Colors = std::vector<CGLColor>;

  static ShaderProgram* s_program;
  static Shape3DObjMgr* s_objectMgr;

  CGLColor color_ { 1.0, 1.0, 1.0, 1.0 };

  ShapeType   shapeType_ { ShapeType::NONE };
  Shape3DData shapeData_;

  Colors colors_;
  bool   wireframe_ { false };

  QString      textureFile_;
  CQGLTexture *diffuseTexture_ { nullptr };
  CQGLTexture *normalTexture_  { nullptr };

  bool useDiffuseTexture_ { false };
  bool useNormalTexture_  { false };
};

}

#endif
