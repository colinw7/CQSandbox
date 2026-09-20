#ifndef CQSandboxRect2DObj_H
#define CQSandboxRect2DObj_H

#include <CQSandboxObject2D.h>

#include <QPainterPath>

namespace CQSandbox {

class Rect2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  Rect2DObj(Canvas2D *canvas, const Rect2D &rect);

  const char *typeName() const override { return "rect"; }

  //---

  const AnimatePoint2D &position() const override;
  void setPosition(const AnimatePoint2D &p) override;

  //---

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect2D calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Rect2D rect_;
};

}

#endif
