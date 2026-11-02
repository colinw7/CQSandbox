#ifndef CQSandboxVector3DObj_H
#define CQSandboxVector3DObj_H

#include <CQSandboxObject3D.h>

namespace CQSandbox {

class Vector2Obj3D : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  Vector2Obj3D(Canvas3D *canvas);

  const char *typeName() const override { return "vector2"; }

  const CVector2D &vector() const { return v_; }
  void setVector(const CVector2D &v) { v_ = v; }

  //---

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  //---

  void init() override;

  CBBox3D calcBBox() override { return bbox_; }

  //---

  void loadBuffer() override { }

  void render() override { }

 protected:
  CVector2D v_;
};

//---

class Vector3Obj3D : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  Vector3Obj3D(Canvas3D *canvas);

  const char *typeName() const override { return "vector3"; }

  const CVector3D &vector() const { return v_; }
  void setVector(const CVector3D &v) { v_ = v; }

  //---

  bool isTclCmd() const override { return true; }

  bool getTclValue(const QString &name, const TclObjs &objs, Tcl_Obj* &res) override;
  bool setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &objs) override;

  bool execTcl(const QString &op, const TclObjs &objs, Tcl_Obj* &res) override;

  //---

  void init() override;

  CBBox3D calcBBox() override { return bbox_; }

  //---

  void loadBuffer() override { }

  void render() override { }

 protected:
  CVector3D v_;
};

}

#endif
