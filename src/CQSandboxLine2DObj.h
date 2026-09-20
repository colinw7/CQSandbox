#ifndef CQSandboxLine2DObj_H
#define CQSandboxLine2DObj_H

#include <CQSandboxObject2D.h>

#include <QPainterPath>

namespace CQSandbox {

class Line2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  Line2DObj(Canvas2D *canvas, const Point2D &p1, const Point2D &p2);

  const char *typeName() const override { return "line"; }

  //---

  const AnimatePoint2D &position() const override;
  void setPosition(const AnimatePoint2D &p) override;

  //---

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect2D calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Point2D p1_;
  Point2D p2_;
};

}

#endif
