#ifndef CQSandboxSurface3DObj_H
#define CQSandboxSurface3DObj_H

#include <CQSandboxObject3D.h>

#include <CGLVector3D.h>

#ifdef CQSANDBOX_WATER_SURFACE
class CWaterSurface;
#endif

#ifdef CQSANDBOX_FLAG
class CFlag;
#endif

namespace CQSandbox {

class Texture3DObj;

class Surface3DObj : public Object3D {
  Q_OBJECT

 public:
  enum class Component {
    RED,
    GREEN,
    BLUE,
    GRAY
  };

  using Points  = std::vector<CPoint3D>;
  using Normals = std::vector<CVector3D>;
  using Colors  = std::vector<CRGBA>;
  using Indices = std::vector<unsigned int>;
  using TPoints = std::vector<CPoint2D>;

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  static ShaderProgram* s_shaderProgram() { return s_program; }

  //---

  Surface3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "surface"; }

  void init() override;

  ShaderProgram *shaderProgram();

  //---

  const Points  &points () const { return points_ ; }
  const Normals &normals() const { return normals_; }
  const Colors  &colors () const { return colors_ ; }
  const TPoints &tpoints() const { return tpoints_; }
  const Indices &indices() const { return indices_; }

  int nx() const { return nx_; }
  int ny() const { return ny_; }

  //---

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  //---

  void resizePoints();

  void tick() override;

#ifdef CQSANDBOX_WATER_SURFACE
  void updateWaterSurface();
#endif

#ifdef CQSANDBOX_FLAG
  void updateFlag();
#endif

  void initShader();

  void calcNormals();

  CBBox3D calcBBox() override { return bbox_; }

  void loadBuffer() override;

  void render() override;

  void updateTexture();

 private:
  static ShaderProgram* s_program;

  Points  points_;
  Normals normals_;
  Colors  colors_;
  Indices indices_;
  TPoints tpoints_;
  int     nx_ { 0 };
  int     ny_ { 0 };

  Texture3DObj *textureObj_       { nullptr };
  Component     textureComponent_ { Component::RED };
  double        textureScale_     { 1.0 };

  bool wireframe_ { false };

#ifdef CQSANDBOX_WATER_SURFACE
  CWaterSurface *waterSurface_ { nullptr };
#endif

#ifdef CQSANDBOX_FLAG
  CFlag *flag_ { nullptr };
#endif
};

}

#endif
