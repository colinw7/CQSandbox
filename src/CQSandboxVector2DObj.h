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

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  double x() const { return v_.getX(); }
  void setX(double r) { v_.setX(r); }

  double y() const { return v_.getY(); }
  void setY(double r) { v_.setY(r); }

 protected:
  CVector2D v_;
};

}

#endif
