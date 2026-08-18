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

class ShaderProgram;

class Surface3DObj : public Object3D {
  Q_OBJECT

 public:
  using Points  = std::vector<CGLVector3D>;
  using Colors  = std::vector<CGLVector3D>;
  using Indices = std::vector<unsigned int>;

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  Surface3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "Surface"; }

  void init() override;

  const Points  &points () const { return points_ ; }
  const Points  &normals() const { return normals_; }
  const Colors  &colors () const { return colors_ ; }
  const Indices &indices() const { return indices_; }

  int nx() const { return nx_; }
  int ny() const { return ny_; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void resizePoints();

  void tick() override;

#ifdef CQSANDBOX_WATER_SURFACE
  void updateWaterSurface();
#endif

#ifdef CQSANDBOX_FLAG
  void updateFlag();
#endif

  void updateGL();

  void calcNormals();

  void render() override;

 private:
  static ShaderProgram* s_program;

  Points  points_;
  Points  normals_;
  Colors  colors_;
  Indices indices_;
  int     nx_ { 0 };
  int     ny_ { 0 };

  bool wireframe_ { false };

#ifdef CQSANDBOX_WATER_SURFACE
  CWaterSurface *waterSurface_ { nullptr };
#endif

#ifdef CQSANDBOX_FLAG
  CFlag *flag_ { nullptr };
#endif

  unsigned int pointsBufferId_  { 0 };
  unsigned int normalsBufferId_ { 0 };
  unsigned int colorsBufferId_  { 0 };
  unsigned int vertexArrayId_   { 0 };
  unsigned int indBufferId_     { 0 };
};

}

#endif
