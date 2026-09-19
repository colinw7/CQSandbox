#include <CQSandboxAxis2DObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQAxis.h>
#include <CQTclUtil.h>

#include <QPainter>

namespace CQSandbox {

bool
Axis2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  if (args.size() != 2) return false;

  auto *tcl = canvas->tcl();

  Point2D pos;
  if (! Util::stringToPoint2D(tcl, args[0], pos))
    return false;

  Coord len;
  if (! Util::stringToCoord(args[1], len))
    return false;

  auto *obj = new Axis2DObj(canvas, pos, len);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

Axis2DObj::
Axis2DObj(Canvas2D *canvas, const Point2D &pos, const Coord &len) :
 Object2D(canvas, Type::AXIS), pos_(pos), len_(len)
{
  axis_ = new CQAxis;
}

bool
Axis2DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if      (name == "pos")
    value = Util::point2DToString(pos_);
  else if (name == "p2")
    value = Util::coordToString(len_);
  else
    return Object2D::getValue(name, args, value);

  return true;
}

bool
Axis2DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->tcl();

  if      (name == "pos") {
    if (! Util::stringToPoint2D(tcl, value, pos_))
      return false;
  }
  else if (name == "p2") {
    if (! Util::stringToCoord(value, len_))
      return false;
  }
  else if (name == "direction") {
    auto lstr = value.toLower();

    if      (lstr == "horizontal")
      axis_->setDirection(CQAxis::DIR_HORIZONTAL);
    else if (lstr == "vertical")
      axis_->setDirection(CQAxis::DIR_VERTICAL);
  }
  else
    return Object2D::setValue(name, value, args);

  return true;
}

Rect2D
Axis2DObj::
calcRect() const
{
  auto pos1 = pointToWindow(pos_);

  Point2D pos2;

  if (len_.units == Units::PIXEL) {
    auto ppos1 = pointToPixel(pos_).qpoint();

    QPointF ppos2;

    if (axis_->getDirection() == CQAxis::DIR_HORIZONTAL)
      ppos2 = QPointF(ppos1.x() + len_.value, ppos1.y());
    else
      ppos2 = QPointF(ppos1.x(), ppos1.y() + len_.value);

    pos2 = pointToWindow(Point2D::makePixel(ppos2));
  }
  else {
    if (axis_->getDirection() == CQAxis::DIR_HORIZONTAL)
      pos2 = Point2D::makeWindow(pos1.x.value + len_.value, pos1.y.value);
    else
      pos2 = Point2D::makeWindow(pos1.x.value, pos1.y.value + len_.value);
  }

  return Rect2D(pos1, pos2);
}

void
Axis2DObj::
draw(QPainter *painter)
{
  painter->setPen(pen_);
  painter->setBrush(brush_.value());

  auto rect  = calcRect();
  auto prect = canvas()->rectToPixel(rect).qrect();

  if (axis_->getDirection() == CQAxis::DIR_HORIZONTAL)
    axis_->draw(painter, prect.left(), prect.bottom(), prect.width());
  else
    axis_->draw(painter, prect.left(), prect.bottom(), -prect.height());
}

}
