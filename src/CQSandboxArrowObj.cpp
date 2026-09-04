#include <CQSandboxArrowObj.h>
#include <CQSandboxCanvas.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQArrow.h>
#include <CQTclUtil.h>

#include <QPainter>

namespace CQSandbox {

bool
ArrowObj::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() != 2) return false;

  auto *tcl = canvas->tcl();

  Point p1, p2;
  if (! Util::stringToPoint(tcl, args[0], p1) ||
      ! Util::stringToPoint(tcl, args[1], p2))
    return false;

  auto *obj = new ArrowObj(canvas, p1, p2);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

ArrowObj::
ArrowObj(Canvas *canvas, const Point &p1, const Point &p2) :
 Object(canvas), p1_(p1), p2_(p2)
{
  arrow_ = new CQArrow;
}

bool
ArrowObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if      (name == "p1")
    value = Util::pointToString(p1_);
  else if (name == "p2")
    value = Util::pointToString(p2_);
  else
    return Object::getValue(name, args, value);

  return true;
}

bool
ArrowObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->tcl();

  if      (name == "p1") {
    if (! Util::stringToPoint(tcl, value, p1_))
      return false;
  }
  else if (name == "p2") {
    if (! Util::stringToPoint(tcl, value, p2_))
      return false;
  }
  else if (name == "lineWidth")
    arrow_->setLineWidth(Util::stringToReal(value));
  else if (name == "front.visible")
    arrow_->setFHead(Util::stringToBool(value));
  else if (name == "front.angle")
    arrow_->setFrontAngle(Util::stringToReal(value));
  else if (name == "front.backAngle")
    arrow_->setFrontBackAngle(Util::stringToReal(value));
  else if (name == "front.length")
    arrow_->setFrontLength(Util::stringToReal(value));
  else if (name == "front.lineEnds")
    arrow_->setFrontLineEnds(Util::stringToBool(value));
  else if (name == "tail.visible")
    arrow_->setTHead(Util::stringToBool(value));
  else if (name == "tail.angle")
    arrow_->setTailAngle(Util::stringToReal(value));
  else if (name == "tail.backAngle")
    arrow_->setTailBackAngle(Util::stringToReal(value));
  else if (name == "tail.length")
    arrow_->setTailLength(Util::stringToReal(value));
  else if (name == "tail.lineEnds")
    arrow_->setTailLineEnds(Util::stringToBool(value));
  else if (name == "filled")
    arrow_->setFilled(Util::stringToBool(value));
  else if (name == "stroked")
    arrow_->setStroked(Util::stringToBool(value));
  else
    return Object::setValue(name, value, args);

  return true;
}

Rect
ArrowObj::
calcRect() const
{
  return Rect(p1_, p2_);
}

void
ArrowObj::
draw(QPainter *painter)
{
  painter->setPen(pen_);
  painter->setBrush(brush_.value());

  class Device : public CQArrowDevice {
   public:
    Device(Canvas *canvas) :
     canvas_(canvas) {
    }

    QPointF windowToPixel(const QPointF &w) override {
      return canvas_->pointToPixel(Point::makeWindow(w)).qpoint();
    }

    QPointF pixelToWindow(const QPointF &p) override {
      return canvas_->pointToWindow(Point::makePixel(p)).qpoint();
    }

   private:
    Canvas *canvas_ { nullptr };
  };

  Device device(canvas());

  auto p1 = pointToWindow(p1_).qpoint();
  auto p2 = pointToWindow(p2_).qpoint();

  arrow_->setFrom(p1);
  arrow_->setTo  (p2);

  arrow_->draw(painter, &device);
}

}
