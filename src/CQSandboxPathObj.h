#ifndef CQSandboxPathObj_H
#define CQSandboxPathObj_H

#include <CQSandboxObject.h>

#include <QPainterPath>

namespace CQSandbox {

class PathObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  PathObj(Canvas *canvas, const QPainterPath &path);

  const char *typeName() const override { return "path"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  bool exec(const QString &op, const QStringList &args, QVariant &res) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

 protected:
  QPainterPath path_;
};

}
#endif
