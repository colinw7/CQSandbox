#ifndef CQSandboxAxis2DObj_H
#define CQSandboxAxis2DObj_H

#include <CQSandboxObject2D.h>

class CQAxis;

namespace CQSandbox {

class Axis2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  Axis2DObj(Canvas2D *canvas, const Point2D &pos, const Coord &len);

  const char *typeName() const override { return "axis"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect2D calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Point2D pos_;
  Coord   len_;

  CQAxis *axis_ { nullptr };
};

}

#endif
