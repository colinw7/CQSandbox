#ifndef CQSandboxPath2DObj_H
#define CQSandboxPath2DObj_H

#include <CQSandboxObject2D.h>

#include <QPainterPath>

namespace CQSandbox {

class Path2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  Path2DObj(Canvas2D *canvas, const QPainterPath &path);

  const char *typeName() const override { return "path"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  bool exec(const QString &op, const QStringList &args, QVariant &res) override;

  Rect2D calcRect() const override;

  void draw(QPainter *) override;

 protected:
  QPainterPath path_;
};

}

#endif
