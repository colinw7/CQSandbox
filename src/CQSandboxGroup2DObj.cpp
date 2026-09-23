#include <CQSandboxGroup2DObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <QPainter>

namespace CQSandbox {

bool
Group2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  if (args.size() != 1) return false;

  auto *tcl = canvas->tcl();

  Rect2D rect;
  if (! Util::stringToRect2D(tcl, args[0], rect))
    return false;

  auto *obj = new Group2DObj(canvas, rect);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

Group2DObj::
Group2DObj(Canvas2D *canvas, const Rect2D &rect) :
 Object2D(canvas, Type::GROUP), rect_(rect)
{
}

bool
Group2DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  auto *tcl = canvas()->tcl();

  if      (name == "rect")
    value = Util::rect2DToString(calcRect());
  else if (name == "range")
    value = Util::rangeToString(tcl, displayRange_);
  else
    return Object2D::getValue(name, args, value);

  return true;
}

bool
Group2DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->tcl();

  if      (name == "rect") {
    if (! Util::stringToRect2D(tcl, value, rect_))
      return false;
  }
  else if (name == "range") {
    if (! Util::stringToRange(tcl, displayRange_, value))
      return false;
  }
  else
    return Object2D::setValue(name, value, args);

  return true;
}

Rect2D
Group2DObj::
calcRect() const
{
  return rect_;
}

void
Group2DObj::
draw(QPainter *painter)
{
  auto rect  = rectToWindow(rect_);
  auto prect = canvas()->rectToPixel(rect).qrect();

  painter->setPen(pen_);
  painter->setBrush(brush_.value());

  painter->drawRect(prect);

  painter->save();
  painter->setClipRect(prect);

  auto qrect = rect.qrect();

  displayRange_.setPixelRange(qrect.left(), qrect.bottom(), qrect.right(), qrect.top());

  for (auto *obj : objects_) {
    if (obj->isVisible())
      obj->draw(painter);
  }

  painter->restore();
}

void
Group2DObj::
addObject(Object2D *obj)
{
  objects_.push_back(obj);

  obj->setGroup(this);

  Q_EMIT objectsChanged();
}

void
Group2DObj::
removeObject(Object2D *obj)
{
  Objects objects;

  for (auto *obj1 : objects_) {
    if (obj1 != obj)
      objects.push_back(obj1);
  }

  std::swap(objects, objects_);

  Q_EMIT objectsChanged();
}

Rect2D
Group2DObj::
rectToPixel(const Rect2D &r) const
{
  auto p1 = pointToPixel(r.ll);
  auto p2 = pointToPixel(r.ur);

  return Rect2D(p1, p2);
}

Point2D
Group2DObj::
pointToPixel(const Point2D &p) const
{
  if (p.x.units == Units::PIXEL)
    return p;

  double px, py;
  displayRange_.windowToPixel(p.x.value, p.y.value, &px, &py);

  auto p1 = Point2D::makePixel(px, py);

  if (group_)
    return group_->pointToPixel(p1);
  else
    return canvas_->pointToPixel(p1);
}

}
