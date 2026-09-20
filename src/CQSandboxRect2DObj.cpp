#include <CQSandboxRect2DObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

#include <QPainter>

namespace CQSandbox {

bool
Rect2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  auto *tcl = canvas->tcl();

  auto rect = Rect2D(Point2D(0, 0), Point2D(1, 1));

  if (args.size() >= 1)
    rect = Util::stringToRect2D(tcl, args[0]);

  auto *obj = new Rect2DObj(canvas, rect);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

Rect2DObj::
Rect2DObj(Canvas2D *canvas, const Rect2D &rect) :
 Object2D(canvas, Type::RECT), rect_(rect)
{
}

bool
Rect2DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if (name == "rect")
    value = Util::rect2DToString(calcRect());
  else
    return Object2D::getValue(name, args, value);

  return true;
}

bool
Rect2DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->tcl();

  if (name == "rect") {
    rect_ = Util::stringToRect2D(tcl, value);
  }
  else
    return Object2D::setValue(name, value, args);

  return true;
}

Rect2D
Rect2DObj::
calcRect() const
{
  return rectToWindow(rect_);
}

void
Rect2DObj::
draw(QPainter *painter)
{
  auto rect  = this->calcRect();
  auto prect = canvas()->rectToPixel(rect).qrect();

  painter->setPen(pen_);
  painter->setBrush(brush_.value());

  painter->drawRect(prect);

  if (isSelected()) {
    painter->setBrush(canvas()->selectedColor());

    painter->drawRect(prect);
  }
}

}
