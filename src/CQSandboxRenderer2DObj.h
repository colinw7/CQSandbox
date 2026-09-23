#ifndef CQSandboxRenderer2DObj_H
#define CQSandboxRenderer2DObj_H

#include <CQSandboxObject2D.h>

#include <QPainter>
#include <QFont>

namespace CQSandbox {

class Renderer2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  Renderer2DObj(Canvas2D *canvas);

  const char *typeName() const override { return "renderer"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  bool exec(const QString &op, const QStringList &args, QVariant &res) override;

 private:
  QPainter *getPainter() const;

  Rect2D getRect() const;

 private:
  Rect2D       rect_;
  bool         rectSet_ { false };
  QBrush       brush_;
  QPen         pen_;
  QFont        font_;
  QImage       image_;
  QPainter*    painter_ { nullptr };
  QPainterPath path_;
};

}

#endif
