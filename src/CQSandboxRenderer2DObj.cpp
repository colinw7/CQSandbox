#include <CQSandboxRenderer2DObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQImageFilter.h>
#include <CQTclUtil.h>

namespace CQSandbox {

bool
Renderer2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  auto *tcl = canvas->tcl();

  auto *obj = new Renderer2DObj(canvas);

  auto name = canvas->addNewObject(obj);

  if (args.size() > 0) {
    if (! Util::stringToRect2D(tcl, args[0], obj->rect_))
      return false;

    obj->rectSet_ = true;
  }

  tcl->setResult(name);

  return true;
}

Renderer2DObj::
Renderer2DObj(Canvas2D *canvas) :
 Object2D(canvas, Type::RENDERER)
{
}

bool
Renderer2DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  //auto *tcl = canvas()->tcl();

  if      (name == "brush.color") {
    value = Util::colorToString(brush_.color());
  }
  else if (name == "pen.color") {
    value = Util::colorToString(pen_.color());
  }
  else if (name == "font.height") {
    QFontMetrics fm(font_);

    value = fm.height();
  }
  else
    return Object2D::getValue(name, args, value);

  return true;
}

bool
Renderer2DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->tcl();

  if      (name == "brush.color") {
    QColor c;
    if (! Util::stringToColor(tcl, value, c))
      return false;

    brush_ = QBrush(c);
  }
  else if (name == "pen.color") {
    QColor c;
    if (! Util::stringToColor(tcl, value, c))
      return false;

    pen_.setColor(c);
  }
  else if (name == "pen.width") {
    double w;
    if (! Util::stringToReal(value, w))
      return false;

    pen_.setWidthF(w);
  }
  else if (name == "font.size") {
    double s;
    if (! Util::stringToReal(value, s))
      return false;

    auto font = font_;

    double scale = 1;

    for (int i = 0; i < 8; ++i) {
      font.setPointSizeF(scale*s);

      QFontMetricsF fm(font);

      double s1 = fm.height();

      scale *= s/s1;
    }

    font_ = font;
  }
  else if (name == "size") {
    if (value == "" || value == "none") {
      rectSet_ = false;
    }
    else {
      Point2D size;
      if (! Util::stringToPoint2D(tcl, value, size))
        return false;

      rect_ = Rect2D(0, 0, size.x.value, size.y.value);

      rectSet_ = true;
    }
  }
  else if (name == "rect") {
    if (value == "" || value == "none") {
      rectSet_ = false;
    }
    else {
      if (! Util::stringToRect2D(tcl, value, rect_))
        return false;

      rectSet_ = true;
    }
  }
  else
    return Object2D::setValue(name, value, args);

  return true;
}

bool
Renderer2DObj::
exec(const QString &op, const QStringList &args, QVariant &res)
{
  auto *tcl = canvas()->tcl();

  // image painter
  if      (op == "paint.begin") {
    delete painter_;

    auto r = getRect();

    auto pr = canvas()->rectToPixel(r).qrect();

    int w = pr.width ();
    int h = pr.height();

    if (image_.isNull() || w != image_.width() || h != image_.height()) {
      image_ = QImage(w, h, QImage::Format_ARGB32);

      image_.fill(brush_.color());
    }

    painter_ = new QPainter(&image_);
  }
  else if (op == "paint.end") {
    delete painter_;

    painter_ = nullptr;
  }
  else if (op == "paint.draw") {
    if (painter_) {
      delete painter_;

      painter_ = nullptr;
    }

    auto *painter = getPainter();
    if (! painter) return false;

    auto r = getRect();

    auto pr = canvas()->rectToPixel(r).qrect();

    painter->drawImage(pr.left(), pr.top(), image_);
  }
  else if (op == "image.blur") {
    image_ = CQImageFilter::gaussianBlur(image_, 1, 1, 2, 2);
  }
  else if (op == "image.erode") {
    image_ = CQImageFilter::erode(image_);
  }

  else if (op == "draw.point") {
    if (args.size() != 1)
      return false;

    auto *painter = getPainter();
    if (! painter) return false;

    Point2D p;
    if (! Util::stringToPoint2D(tcl, args[0], p))
      return false;

    painter->setPen(pen_);

    auto pp = pointToPixel(p);

    painter->drawPoint(pp.x.value, pp.y.value);
  }
  else if (op == "draw.rect") {
    auto *painter = getPainter();
    if (! painter) return false;

    Rect2D r;
    if (args.size() >= 1) {
      if (! Util::stringToRect2D(tcl, args[0], r))
        return false;
    }
    else {
      r = getRect();
    }

    painter->setPen(pen_);

    auto pr = canvas()->rectToPixel(r).qrect();

    painter->drawRect(pr);
  }
  else if (op == "fill.rect") {
    auto *painter = getPainter();
    if (! painter) return false;

    Rect2D r;
    if (args.size() >= 1) {
      if (! Util::stringToRect2D(tcl, args[0], r))
        return false;
    }
    else {
      r = getRect();
    }

    auto pr = canvas()->rectToPixel(r).qrect();

    painter->fillRect(pr, brush_);
  }
  else if (op == "draw.ellipse") {
    auto *painter = getPainter();
    if (! painter) return false;

    Rect2D r;
    if (args.size() >= 1) {
      if (! Util::stringToRect2D(tcl, args[0], r))
        return false;
    }
    else {
      r = getRect();
    }

    painter->setPen(pen_);
    painter->setBrush(brush_);

    auto pr = canvas()->rectToPixel(r).qrect();

    painter->drawEllipse(pr);
  }
  else if (op == "draw.text") {
    if (args.size() != 2)
      return false;

    auto *painter = getPainter();
    if (! painter) return false;

    Point2D p;
    if (! Util::stringToPoint2D(tcl, args[0], p))
      return false;

    auto text = args[1];

    painter->setPen(pen_);

    auto pp = pointToPixel(p);

    painter->setFont(font_);

    painter->drawText(pp.x.value, pp.y.value, text);
  }
  else if (op == "path.start") {
    path_ = QPainterPath();
  }
  else if (op == "path.moveTo") {
    if (args.size() != 1)
      return false;

    Point2D p;
    if (! Util::stringToPoint2D(tcl, args[0], p))
      return false;

    auto pp = pointToPixel(p);

    path_.moveTo(pp.x.value, pp.y.value);
  }
  else if (op == "path.curveTo") {
    if (args.size() != 3)
      return false;

    Point2D p1, p2, p3;
    if (! Util::stringToPoint2D(tcl, args[0], p1) ||
        ! Util::stringToPoint2D(tcl, args[1], p2) ||
        ! Util::stringToPoint2D(tcl, args[2], p3))
      return false;

    auto pp1 = pointToPixel(p1);
    auto pp2 = pointToPixel(p2);
    auto pp3 = pointToPixel(p3);

    path_.cubicTo(pp1.x.value, pp1.y.value,
                  pp2.x.value, pp2.y.value,
                  pp3.x.value, pp3.y.value);
  }
  else if (op == "path.draw") {
    auto *painter = getPainter();
    if (! painter) return false;

    painter->setPen(pen_);
    painter->setBrush(brush_);

    painter->drawPath(path_);
  }
  else
    return Object2D::exec(op, args, res);

  return true;
}

Rect2D
Renderer2DObj::
getRect() const
{
  if (rectSet_)
    return rect_;
  else
    return Rect2D(0, 0, canvas()->width(), canvas()->height());
}

QPainter *
Renderer2DObj::
getPainter() const
{
  if (painter_)
    return painter_;

  return canvas()->painter();
}

}
