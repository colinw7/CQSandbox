#ifndef CQSandboxShape3DObj_H
#define CQSandboxShape3DObj_H

#include <CQSandboxObject3D.h>
#include <CQSandboxShape3DData.h>

#include <CGLVector2D.h>
#include <CGLColor.h>

class CShape3D;

namespace CQSandbox {

class ShaderProgram;
class Path3DObj;
class Texture;

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
    SPHERE,
    TORUS,
    GEAR,
    PATH
  };

  //---

  using ConfigData = Shape3DData::ConfigData;

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  static ShaderProgram* shaderProgram() { return s_program; }

  static void initShader(Canvas3D *canvas);

  static void initDraw(Canvas3D *canvas);
  static void initDraw(Canvas3D *canvas, ShaderProgram *program);
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

  void addCone(double r, double h, const ConfigData &configData=ConfigData());
  void addCube(double sx, double sy, double sz);
  void addCylinder(double r, double h, const ConfigData &configData=ConfigData());
  void addSphere(double r, const ConfigData &configData=ConfigData());
  void addTorus(double r0, double r1, double power1, double power2,
                const ConfigData &configData=ConfigData());
  void addGear(double r, int numTeeth, double toothSize, double rot);
  void addPath(Path3DObj *, double, double);

  //---

  void setDiffuseColor(const QColor &c);

  //---

  QString diffuseTextureFile() const;
  bool setDiffuseTextureFile(const QString &filename);

  void setDiffuseTexture(Texture *texture);

  QString normalTextureFile() const;
  bool setNormalTextureFile(const QString &filename);

  void setNormalTexture(Texture *texture);

  QString specularTextureFile() const;
  bool setSpecularTextureFile(const QString &filename);

  void setSpecularTexture(Texture *texture);

  QString emissionTextureFile() const;
  bool setEmissionTextureFile(const QString &filename);

  void setEmissionTexture(Texture *texture);

  //---

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  bool intersectRay(const CVector3D &p1, const CVector3D &p2,
                    CPoint3D &pi1, CPoint3D &pi2) const override;

  CBBox3D calcBBox() override;

  const FaceDatas  &getFaceDatas () const override;
  const LineDatas  &getLineDatas () const override;
  const PointDatas &getPointDatas() const override;

  void loadBuffer() override;

  void render() override;

  void render(ShaderProgram *program);

  //---

  bool setPointPosition(int ind, const CPoint3D &p) override;

  //---

  void applyMatrix(const CMatrix3DH &m) override;

 private:
  Material3D *initMaterial();

 protected:
  using Normals   = std::vector<CVector3D>;
  using TexCoords = std::vector<CVector2D>;
  using Colors    = std::vector<CGLColor>;

  static ShaderProgram* s_program;
  static Shape3DObjMgr* s_objectMgr;
  static CMatrix3DH     s_pvMatrix;

  ShapeType   shapeType_ { ShapeType::NONE };
  Shape3DData shapeData_;

  ConfigData configData_;

  Colors colors_;
  bool   wireframe_ { false };

  Normals   s_normals_;
  TexCoords s_texCoords_;
  Colors    s_colors_;
};

}

#endif
