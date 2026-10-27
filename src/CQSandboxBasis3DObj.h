#ifndef CQSandboxBasis3DObj_H
#define CQSandboxBasis3DObj_H

#include <CQSandboxObject3D.h>
#include <CGLVector3D.h>

namespace CQSandbox {

class Shape3DObj;

class Basis3DObj : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  Basis3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "basis"; }

  //---

  Object3D *parentObj() const { return parentObj_; }
  void setParentObj(Object3D *o);

  //---

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void loadBuffer() override;

  CBBox3D calcBBox() override;

  void render() override;

 protected:
  Object3D* parentObj_ { nullptr };

  Shape3DObj* xCylinder_ { nullptr };
  Shape3DObj* yCylinder_ { nullptr };
  Shape3DObj* zCylinder_ { nullptr };
};

}

#endif
