#ifndef CQSandboxGrid3DObj_H
#define CQSandboxGrid3DObj_H

#include <CQSandboxObject3D.h>

namespace CQSandbox {

class ShaderProgram;

class Grid3DObj : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  Grid3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "grid"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void initShader();

  CBBox3D calcBBox() override { return bbox_; }

  void updateGL();

  void render() override;

 private:
  static ShaderProgram* s_program;
};

}

#endif
