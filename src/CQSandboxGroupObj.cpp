#include <CQSandboxGroupObj.h>
#include <CQSandboxCanvas.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <QPainter>

namespace CQSandbox {

bool
GroupObj::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() != 1) return false;

  auto *tcl = canvas->tcl();

  auto rect = Util::stringToRect(tcl, args[0]);

  auto *obj = new GroupObj(canvas, rect);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

GroupObj::
GroupObj(Canvas *canvas, const Rect &rect) :
 Object(canvas), rect_(rect)
{
}

bool
GroupObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  auto *tcl = canvas()->tcl();

  if      (name == "rect")
    value = Util::rectToString(calcRect());
  else if (name == "range")
    value = Util::rangeToString(tcl, displayRange_);
  else
    return Object::getValue(name, args, value);

  return true;
}

bool
GroupObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->tcl();

  if      (name == "rect")
    rect_ = Util::stringToRect(tcl, value);
  else if (name == "range")
    Util::stringToRange(tcl, displayRange_, value);
  else
    return Object::setValue(name, value, args);

  return true;
}

Rect
GroupObj::
calcRect() const
{
  return rect_;
}

void
GroupObj::
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
GroupObj::
addObject(Object *obj)
{
  objects_.push_back(obj);

  obj->setGroup(this);

  Q_EMIT objectsChanged();
}

void
GroupObj::
removeObject(Object *obj)
{
  Objects objects;

  for (auto *obj1 : objects_) {
    if (obj1 != obj)
      objects.push_back(obj1);
  }

  std::swap(objects, objects_);

  Q_EMIT objectsChanged();
}

Rect
GroupObj::
rectToPixel(const Rect &r) const
{
  auto p1 = pointToPixel(r.ll);
  auto p2 = pointToPixel(r.ur);

  return Rect(p1, p2);
}

Point
GroupObj::
pointToPixel(const Point &p) const
{
  if (p.x.units == Units::PIXEL)
    return p;

  double px, py;
  displayRange_.windowToPixel(p.x.value, p.y.value, &px, &py);

  auto p1 = Point::makePixel(px, py);

  if (group_)
    return group_->pointToPixel(p1);
  else
    return canvas_->pointToPixel(p1);
}

}
