#ifndef CQSandboxCircle2DObj_H
#define CQSandboxCircle2DObj_H

#include <CQSandboxObject2D.h>

#include <QPainterPath>

namespace CQSandbox {

class Circle2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  Circle2DObj(Canvas2D *canvas, const CPoint2D &center, const Coord &radius);

  const char *typeName() const override { return "circle"; }

  const AnimatePoint2D &center() const { return position(); }
  void setCenter(const AnimatePoint2D &c) { setPosition(c); }

  void setTargetCenter(const CPoint2D &c) { position_.setTarget(c); }

  const AnimateCoord &radius() const { return radius_; }
  void setRadius(const AnimateCoord &r) { radius_ = r; }
  void setTargetRadius(const Coord &r) { radius_.setTarget(r); }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect2D calcRect() const override;

  bool step() override;

  void draw(QPainter *) override;

 protected:
  AnimateCoord   radius_;
};

}

#endif
