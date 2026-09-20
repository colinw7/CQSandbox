#include <CQSandboxCircle2DObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

#include <QPainter>

namespace CQSandbox {

bool
Circle2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  if (args.size() != 2) return false;

  auto *tcl = canvas->tcl();

  Point2D center;
  if (! Util::stringToPoint2D(tcl, args[0], center))
    return false;

  Coord r;
  if (! Util::stringToCoord(args[1], r))
    return false;

  //---

  auto center1 = canvas->pointToWindow(center).point();

  auto *obj = new Circle2DObj(canvas, center1, r);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

Circle2DObj::
Circle2DObj(Canvas2D *canvas, const CPoint2D &center, const Coord &radius) :
 Object2D(canvas, Type::CIRCLE), radius_(radius)
{
  position_ = center;
}

bool
Circle2DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if      (name == "rect")
    value = Util::rect2DToString(calcRect());
  else if (name == "center")
    value = Util::point2DToString(Point2D::makeWindow(position_.value()));
  else if (name == "center.target")
    value = Util::point2DToString(position_.target());
  else if (name == "center.steps")
    value = int(position_.steps());
  else if (name == "radius")
    value = Util::coordToString(radius_.value());
  else if (name == "radius.target")
    value = Util::coordToString(radius_.target());
  else if (name == "radius.steps")
    value = int(radius_.steps());
  else
    return Object2D::getValue(name, args, value);

  return true;
}

bool
Circle2DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->tcl();

  if      (name == "center") {
    Point2D p;
    if (! Util::stringToPoint2D(tcl, value, p))
      return false;

    position_.setValue(canvas()->pointToWindow(p).point());
  }
  else if (name == "center.target") {
    Point2D p;
    if (! Util::stringToPoint2D(tcl, value, p))
      return false;

    position_.setTarget(canvas()->pointToWindow(p).point());
  }
  else if (name == "center.steps") {
    position_.setSteps(Util::stringToInt(value));
  }
  else if (name == "radius") {
    Coord c;
    if (! Util::stringToCoord(value, c))
      return false;

    radius_.setValue(c);
  }
  else if (name == "radius.target") {
    Coord c;
    if (! Util::stringToCoord(value, c))
      return false;

    radius_.setTarget(c);
  }
  else if (name == "radius.steps") {
    radius_.setSteps(Util::stringToInt(value));
  }
  else
    return Object2D::setValue(name, value, args);

  return true;
}

Rect2D
Circle2DObj::
calcRect() const
{
  auto c = pointToWindow(Point2D::makeWindow(position_.value())).point();

  auto radius = radius_.value();

  double xr = radius.value;
  double yr = xr;

  if (radius.units == Units::PIXEL) {
    auto p1 = canvas()->pointToWindow(Point2D::makePixel(0.0, 0.0));
    auto p2 = canvas()->pointToWindow(Point2D::makePixel(xr, yr));

    xr = std::abs(p2.x.value - p1.x.value);
    yr = std::abs(p2.y.value - p1.y.value);
  }

  auto ll = Point2D::makeWindow(c.x - xr, c.y - yr);
  auto ur = Point2D::makeWindow(c.x + xr, c.y + yr);

  return Rect2D(ll, ur);
}

bool
Circle2DObj::
step()
{
  bool b1 = position_.step();
  bool b2 = radius_.step();
  bool b3 = Object2D::step();

  return (b1 || b2 || b3);
}

void
Circle2DObj::
draw(QPainter *painter)
{
  auto rect  = this->calcRect();
  auto prect = canvas()->rectToPixel(rect).qrect();

  painter->setPen(pen_);
  painter->setBrush(brush_.value());

  painter->drawEllipse(prect);

  if (isSelected()) {
    painter->setBrush(QColor(255, 255, 0, 100));

    painter->drawEllipse(prect);
  }
}

}
