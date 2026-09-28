#ifndef CQSandboxVector2DObj_H
#define CQSandboxVector2DObj_H

#include <CQSandboxObject2D.h>

#include <CVector2D.h>

namespace CQSandbox {

class Vector2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  Vector2DObj(Canvas2D *canvas);

  const char *typeName() const override { return "vector"; }

  bool isTclCmd() const override { return true; }

  double x() const { return v_.getX(); }
  void setX(double r) { v_.setX(r); }

  double y() const { return v_.getY(); }
  void setY(double r) { v_.setY(r); }

  //---

  bool getTclValue(const QString &name, const TclObjs &objs, Tcl_Obj* &res) override;
  bool setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &objs) override;

  bool execTcl(const QString &op, const TclObjs &objs, Tcl_Obj* &res) override;

  //---

  bool isDrawable() const override { return false; }

  void draw(QPainter *) override { }

 protected:
  CVector2D v_;
};

}

#endif
