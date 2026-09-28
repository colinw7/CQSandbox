#ifndef CQSandboxColor2DObj_H
#define CQSandboxColor2DObj_H

#include <CQSandboxObject2D.h>

#include <CRGBA.h>

namespace CQSandbox {

class Color2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  Color2DObj(Canvas2D *canvas);

  const char *typeName() const override { return "color"; }

  const CRGBA &color() const { return c_; }
  void setColor(const CRGBA &c) { c_ = c; }

  //---

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  //---

  bool isDrawable() const override { return false; }

  void draw(QPainter *) override { }

 protected:
  CRGBA c_;
};

}

#endif
