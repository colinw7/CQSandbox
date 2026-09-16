#include <CQSandboxRenderer2DObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

bool
Renderer2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  if (args.size() != 0)
    return false;

  auto *tcl = canvas->tcl();

  auto *obj = new Renderer2DObj(canvas);

  auto name = canvas->addNewObject(obj);

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

    brush_.setColor(c);
  }
  else if (name == "pen.color") {
    QColor c;
    if (! Util::stringToColor(tcl, value, c))
      return false;

    pen_.setColor(c);
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
  else
    return Object2D::setValue(name, value, args);

  return true;
}

bool
Renderer2DObj::
exec(const QString &op, const QStringList &args, QVariant &res)
{
  auto *tcl = canvas()->tcl();

  if      (op == "draw.point") {
    if (args.size() != 1)
      return false;

    auto *painter = canvas()->painter();
    if (! painter) return false;

    Point2D p;
    if (! Util::stringToPoint2D(tcl, args[0], p))
      return false;

    painter->setPen(pen_);

    auto pp = pointToPixel(p);

    painter->drawPoint(pp.x.value, pp.y.value);

    return true;
  }
  else if (op == "draw.rect") {
    if (args.size() != 1)
      return false;

    auto *painter = canvas()->painter();
    if (! painter) return false;

    auto r = Util::stringToRect2D(tcl, args[0]);

    painter->setPen(pen_);

    auto pr = canvas()->rectToPixel(r).qrect();

    painter->drawRect(pr);

    return true;
  }
  else if (op == "draw.text") {
    if (args.size() != 2)
      return false;

    auto *painter = canvas()->painter();
    if (! painter) return false;

    Point2D p;
    if (! Util::stringToPoint2D(tcl, args[0], p))
      return false;

    auto text = args[1];

    painter->setPen(pen_);

    auto pp = pointToPixel(p);

    painter->setFont(font_);

    painter->drawText(pp.x.value, pp.y.value, text);

    return true;
  }
  else
    return Object2D::exec(op, args, res);

  return false;
}

}
