#include <CQSandboxPointListObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <QPainter>

namespace CQSandbox {

bool
PointListObj::
create(Canvas2D *canvas, const QStringList &args)
{
  if (args.size() != 1) return false;

  auto *tcl = canvas->tcl();

  Coord r;
  if (! Util::stringToCoord(args[0], r))
    return false;

  auto *obj = new PointListObj(canvas, r);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

PointListObj::
PointListObj(Canvas2D *canvas, const Coord &radius) :
 Object2D(canvas, Type::POINT_LIST), radius_(radius)
{
}

bool
PointListObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if      (name == "radius")
    value = Util::coordToString(radius_.value());
  else if (name == "radius.target")
    value = Util::coordToString(radius_.target());
  else if (name == "radius.steps")
    value = int(radius_.steps());
  else if (name == "size")
    value = Util::intToString(int(points_.size()));
  else if (name == "connected")
    value = Util::boolToString(isConnected());
  else if (name == "show_points")
    value = Util::boolToString(isShowPoints());
  else if (name == "angle")
    value = angle();
  else if (name == "scale")
    value = scale();
  else if (name == "offset")
    value = Util::point2DToString(offset());
  else if (name == "fill_under")
    value = Util::boolToString(isFillUnder());
  else if (name == "fill_under.y") {
    if (fillUnderY())
      value = Util::coordToString(*fillUnderY());
    else
      value = QVariant();
  }
  else if (name == "position") {
    if (args.size() > 0) {
      auto i = Util::stringToInt(args[0]);

      if (i < 0 || i >= int(points_.size()))
        return false;

      value = Util::point2DToString(points_[i]);
    }
    else
      return false;
  }
  else if (name == "intersect") {
    if (args.size() < 0)
      return false;

    Point2D pos;
    if (! Util::stringToPoint2D(tcl, args[0], pos))
      return false;

    auto pos1 = canvas()->pointToPixel(pos).qpoint();

    value = path_.contains(pos1);
  }
  else if (name == "intersect_obj") {
    if (args.size() < 0)
      return false;

    auto *obj = canvas()->getObjectByName(args[0]);
    if (! obj)
      return app->errorMsg(QString("Failed to find object '%1'").arg(args[0]));

    auto path1 = calcPath();
    auto path2 = obj->calcPath();

    value = path1.intersects(path2);
  }
  else
    return Object2D::getValue(name, args, value);

  return true;
}

bool
PointListObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if      (name == "radius") {
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
  else if (name == "radius.steps")
    radius_.setSteps(Util::stringToInt(value));
  else if (name == "size") {
    auto n = Util::stringToInt(value);

    auto n1 = int(points_.size());

    if      (n > n1) {
      for (int i = 0; i < n - n1; ++i)
        points_.emplace_back();
    }
    else if (n < n1) {
      for (int i = 0; i < n1 - n; ++i)
        points_.pop_back();
    }
  }
  else if (name == "connected")
    setConnected(Util::stringToBool(value));
  else if (name == "show_points")
    setFillUnder(Util::stringToBool(value));
  else if (name == "fill_under")
    setFillUnder(Util::stringToBool(value));
  else if (name == "fill_under.y") {
    Coord c;
    if (! Util::stringToCoord(value, c))
      return false;

    setFillUnderY(c);
  }
  else if (name == "angle")
    setAngle(Util::stringToReal(value));
  else if (name == "scale")
    setScale(Util::stringToReal(value));
  else if (name == "offset") {
    Point2D p;
    if (! Util::stringToPoint2D(tcl, value, p))
      return false;

    setOffset(p);
  }
  else if (name == "position") {
    // get index from args
    if (args.size() > 0) {
      auto i = Util::stringToInt(args[0]);

      if (i < 0 || i >= int(points_.size()))
        return false;

      if (! Util::stringToPoint2D(tcl, value, points_[i]))
        return false;
    }
    else
      return app->errorMsg("Missing index for position");
  }
  else
    return Object2D::setValue(name, value, args);

  return true;
}

Rect2D
PointListObj::
calcRect() const
{
  QRectF r;
  bool   rset { false };

  for (const auto &point : points_) {
    auto c = pointToWindow(point).qpoint();

    if (! rset) {
      r = QRectF(c.x(), c.y(), 0, 0);

      rset = true;
    }
    else {
      auto x1 = std::min(r.left  (), c.x());
      auto y1 = std::min(r.top   (), c.y());
      auto x2 = std::max(r.right (), c.x());
      auto y2 = std::max(r.bottom(), c.y());

      r = QRectF(x1, y1, x2 - x1, y2 - y1);
    }
  }

  return Rect2D::makeWindow(r);
}

bool
PointListObj::
step()
{
  bool b1 = radius_.step();
  bool b2 = Object2D::step();

  return (b1 || b2);
}

void
PointListObj::
draw(QPainter *painter)
{
  auto rect  = this->calcRect();
  auto prect = canvas()->rectToPixel(rect).qrect();

  auto c  = center();
  auto pc = canvas()->pointToPixel(c).qpoint();

  auto po = canvas()->pointToPixel(Point2D()).qpoint();
  auto pf = canvas()->pointToPixel(offset()).qpoint() - po;

  painter->setPen(pen_);
  painter->setBrush(brush_.value());

  QTransform t;

  t.translate(pc.x() + pf.x(), pc.y() + pf.y());
  t.rotate(-angle());
  t.scale(scale(), scale());
  t.translate(-pc.x(), -pc.y());

#if 0
  painter->setTransform(t);
#endif

  //painter->drawRect(prect);

  //---

  auto radius = radius_.value();

  double xr = radius.value;
  double yr = xr;

  if (radius.units == Units::PIXEL) {
    auto p1 = canvas()->pointToWindow(Point2D::makePixel(0.0, 0.0));
    auto p2 = canvas()->pointToWindow(Point2D::makePixel(xr, yr));

    xr = std::abs(p2.x.value - p1.x.value);
    yr = std::abs(p2.y.value - p1.y.value);
  }

  //---

  QPainterPath path;

  int i = 0;

  QPointF p1, p2;

  for (const auto &point : points_) {
    auto p = pointToWindow(point);

    auto pp = canvas()->pointToPixel(p).qpoint();

    if (i == 0) {
      p1 = pp;

      path.moveTo(pp);
    }
    else {
      path.lineTo(pp);

      p2 = pp;
    }

    ++i;
  }

  auto path1 = path;

  if (isConnected())
    path1.closeSubpath();

  path_ = t.map(path1);

  //---

  if (isFillUnder()) {
    auto by = prect.bottom();

    if (fillUnderY()) {
      auto c = *fillUnderY();
      auto p = Point2D(c, c);

      auto pw = pointToWindow(p);
      auto pp = canvas()->pointToPixel(pw).qpoint();

      by = pp.y();
    }

    auto path2 = path;

    path2.lineTo(p2.x(), by);
    path2.lineTo(p1.x(), by);

    path2.closeSubpath();

    painter->fillPath(t.map(path2), painter->brush());
  }

  if (isFilled())
    painter->fillPath(path_, painter->brush());

  if (isStroked())
    painter->strokePath(path_, painter->pen());

  if (isShowPoints()) {
    for (const auto &point : points_) {
      auto c = pointToWindow(point);

      auto ll = Point2D::makeWindow(c.x.value - xr, c.y.value - yr);
      auto ur = Point2D::makeWindow(c.x.value + xr, c.y.value + yr);

      auto rect  = Rect2D(ll, ur);
      auto prect = canvas()->rectToPixel(rect).qrect();

      painter->drawEllipse(prect);
    }
  }
}

}
