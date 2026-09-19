#include <CQSandboxLine2DObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

#include <QPainter>

namespace CQSandbox {

bool
Line2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  if (args.size() != 2) return false;

  auto *tcl = canvas->tcl();

  Point2D p1, p2;

  if (! Util::stringToPoint2D(tcl, args[0], p1) ||
      ! Util::stringToPoint2D(tcl, args[1], p2))
    return false;

  auto *obj = new Line2DObj(canvas, p1, p2);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

Line2DObj::
Line2DObj(Canvas2D *canvas, const Point2D &p1, const Point2D &p2) :
 Object2D(canvas, Type::LINE), p1_(p1), p2_(p2)
{
}

bool
Line2DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if      (name == "p1")
    value = Util::point2DToString(p1_);
  else if (name == "p2")
    value = Util::point2DToString(p2_);
  else
    return Object2D::getValue(name, args, value);

  return true;
}

bool
Line2DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->tcl();

  if      (name == "p1") {
    if (! Util::stringToPoint2D(tcl, value, p1_))
      return false;
  }
  else if (name == "p2") {
    if (! Util::stringToPoint2D(tcl, value, p2_))
      return false;
  }
  else
    return Object2D::setValue(name, value, args);

  return true;
}

Rect2D
Line2DObj::
calcRect() const
{
  auto p1 = pointToWindow(p1_);
  auto p2 = pointToWindow(p2_);

  return Rect2D(p1, p2);
}

void
Line2DObj::
draw(QPainter *painter)
{
  painter->setPen(pen_);
  painter->setBrush(brush_.value());

  auto p1 = pointToPixel(p1_).qpoint();
  auto p2 = pointToPixel(p2_).qpoint();

  painter->drawLine(p1, p2);
}

}
