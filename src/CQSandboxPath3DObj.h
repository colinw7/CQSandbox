#ifndef CQSandboxPath3DObj_H
#define CQSandboxPath3DObj_H

#include <CQSandboxObject3D.h>

#include <CGLPath3D.h>

class CGLPath3D;

namespace CQSandbox {

class ShaderProgram;

class Path3DObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Path3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "Path"; }

  void setLine(const CGLVector3D &p1, const CGLVector3D &p2);

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void initShader();

  void render() override;

 private:
  void updatePoints();

  void updateGL();

 private:
  CGLPath3D path_;

  using Points = std::vector<CGLVector3D>;

  static ShaderProgram* s_program;

  Points points_;

  unsigned int pointsBufferId_ { 0 };
  unsigned int vertexArrayId_  { 0 };
};

}

#endif
