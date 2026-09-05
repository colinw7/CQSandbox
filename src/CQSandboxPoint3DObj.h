#ifndef CQSandboxPoint3DObj_H
#define CQSandboxPoint3DObj_H

#include <CQSandboxObject3D.h>

namespace CQSandbox {

class ShaderProgram;

class Point3DObj : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  Point3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "point"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void initShader();

  CBBox3D calcBBox() override { return bbox_; }

  void render() override;

 private:
  void updateGL();

 protected:
  static ShaderProgram* s_program;

  double size_  { 3 };
  QColor color_ { Qt::red };
};

}

#endif
