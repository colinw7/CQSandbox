#ifndef CQSandboxArrowObj_H
#define CQSandboxArrowObj_H

#include <CQSandboxObject.h>

class CQArrow;

namespace CQSandbox {

class ArrowObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  ArrowObj(Canvas *canvas, const Point &p1, const Point &p2);

  const char *typeName() const override { return "arrow"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Point p1_;
  Point p2_;

  CQArrow *arrow_ { nullptr };
};

}

#endif
