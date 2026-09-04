#ifndef CQSandboxVector3DObj_H
#define CQSandboxVector3DObj_H

#include <CQSandboxObject3D.h>

namespace CQSandbox {

class Vector3DObj : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  Vector3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "vector"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  //---

  void init() override;

  CBBox3D calcBBox() override { return bbox_; }

 protected:
  CVector2D v_;
};

}

#endif
