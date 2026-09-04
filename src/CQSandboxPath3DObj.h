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
  using Points = std::vector<CVector3D>;

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  Path3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "path"; }

  const CGLPath3D &path() const { return path_; }

  const Points &points() const { return points_; }

  void setLine(const CVector3D &p1, const CVector3D &p2);

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;
  bool exec(const QString &op, const QStringList &args, QVariant &res) override;

  void init() override;

  void initShader();

  CBBox3D calcBBox() override { return bbox_; }

  void render() override;

 private:
  void updatePoints();

  void updateGL();

 protected:
  static ShaderProgram* s_program;

  CGLPath3D path_;

  Points points_;
};

}

#endif
