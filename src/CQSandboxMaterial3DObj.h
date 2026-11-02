#ifndef CQSandboxMaterial3DObj_H
#define CQSandboxMaterial3DObj_H

#include <CQSandboxObject3D.h>

namespace CQSandbox {

class Material3D;

class Material3DObj : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  Material3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "material"; }

  Material3D *material() const override { return material_; }
  void setMaterial(Material3D *p) override { material_ = p; }

  //---

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  //---

  void init() override;

  //---

  void render() override { }

  void loadBuffer() override { }

 private:
  Material3D *material_ { nullptr };
};

}

#endif
