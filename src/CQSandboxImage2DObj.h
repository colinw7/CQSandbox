#ifndef CQSandboxImage2DObj_H
#define CQSandboxImage2DObj_H

#include <CQSandboxObject2D.h>

#include <QPainterPath>

namespace CQSandbox {

class Image2DObj : public Object2D {
  Q_OBJECT

 public:
  enum Position {
    TOP_LEFT,
    CENTER,
    RECT
  };

  static bool create(Canvas2D *canvas, const QStringList &args);

  Image2DObj(Canvas2D *canvas, const CPoint2D &pos, const QImage &image);

  const char *typeName() const override { return "image"; }

  const QImage &image() const { return image_; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  bool exec(const QString &op, const QStringList &args, QVariant &res) override;

  Rect2D calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Rect2D   rect_;
  Position posType_ { Position::TOP_LEFT };
  QImage   image_;
  QImage   imageMask_;
  QPen     pen_;
  QBrush   brush_;
};

}

#endif
