#include <CQSandboxAxisObj.h>
#include <CQSandboxCanvas.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQAxis.h>
#include <CQTclUtil.h>

#include <QPainter>

namespace CQSandbox {

bool
AxisObj::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() != 2) return false;

  auto *tcl = canvas->app()->tcl();

  auto pos = Util::stringToPoint(tcl, args[0]);
  auto len = Util::stringToCoord(args[1]);

  auto *obj = new AxisObj(canvas, pos, len);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

AxisObj::
AxisObj(Canvas *canvas, const Point &pos, const Coord &len) :
 Object(canvas), pos_(pos), len_(len)
{
  axis_ = new CQAxis;
}

bool
AxisObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if      (name == "pos")
    value = Util::pointToString(pos_);
  else if (name == "p2")
    value = Util::coordToString(len_);
  else
    return Object::getValue(name, args, value);

  return true;
}

bool
AxisObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->app()->tcl();

  if      (name == "pos")
    pos_ = Util::stringToPoint(tcl, value);
  else if (name == "p2")
    len_ = Util::stringToCoord(value);
  else if (name == "direction") {
    auto lstr = value.toLower();

    if      (lstr == "horizontal")
      axis_->setDirection(CQAxis::DIR_HORIZONTAL);
    else if (lstr == "vertical")
      axis_->setDirection(CQAxis::DIR_VERTICAL);
  }
  else
    return Object::setValue(name, value, args);

  return true;
}

Rect
AxisObj::
calcRect() const
{
  Point pos1 = pointToWindow(pos_);
  Point pos2;

  if (len_.units == Units::PIXEL) {
    auto ppos1 = pointToPixel(pos_).qpoint();

    QPointF ppos2;

    if (axis_->getDirection() == CQAxis::DIR_HORIZONTAL)
      ppos2 = QPointF(ppos1.x() + len_.value, ppos1.y());
    else
      ppos2 = QPointF(ppos1.x(), ppos1.y() + len_.value);

    pos2 = pointToWindow(Point::makePixel(ppos2));
  }
  else {
    if (axis_->getDirection() == CQAxis::DIR_HORIZONTAL)
      pos2 = Point::makeWindow(pos1.x.value + len_.value, pos1.y.value);
    else
      pos2 = Point::makeWindow(pos1.x.value, pos1.y.value + len_.value);
  }

  return Rect(pos1, pos2);
}

void
AxisObj::
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
