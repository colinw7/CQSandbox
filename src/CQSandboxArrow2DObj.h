#ifndef CQSandboxArrow2DObj_H
#define CQSandboxArrow2DObj_H

#include <CQSandboxObject2D.h>

class CQArrow;

namespace CQSandbox {

class Arrow2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  Arrow2DObj(Canvas2D *canvas, const Point2D &p1, const Point2D &p2);

  const char *typeName() const override { return "arrow"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect2D calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Point2D p1_;
  Point2D p2_;

  CQArrow *arrow_ { nullptr };
};

}

#endif
