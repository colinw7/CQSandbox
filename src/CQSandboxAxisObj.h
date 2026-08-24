#ifndef CQSandboxAxisObj_H
#define CQSandboxAxisObj_H

#include <CQSandboxObject.h>

class CQAxis;

namespace CQSandbox {

class AxisObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  AxisObj(Canvas *canvas, const Point &pos, const Coord &len);

  const char *typeName() const override { return "axis"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Point pos_;
  Coord len_;

  CQAxis *axis_ { nullptr };
};

}

#endif
