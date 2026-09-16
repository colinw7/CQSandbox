#include <CQSandboxVector2DObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

bool
Vector2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  auto *tcl = canvas->tcl();

  auto *obj = new Vector2DObj(canvas);

  if (args.size() >= 2) {
    double x, y;
    if (! Util::stringToReal(args[0], x) || ! Util::stringToReal(args[1], y))
      return false;

    obj->setX(x);
    obj->setY(y);

    return false;
  }

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

Vector2DObj::
Vector2DObj(Canvas2D *canvas) :
 Object2D(canvas, Type::VECTOR)
{
}

bool
Vector2DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if      (name == "x")
    value = v_.x();
  else if (name == "y")
    value = v_.y();
  else
    return Object2D::getValue(name, args, value);

  return true;
}

bool
Vector2DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "x") {
    double r;
    if (! Util::stringToReal(value, r))
      return false;

    setX(r);
  }
  else if (name == "y") {
    double r;
    if (! Util::stringToReal(value, r))
      return false;

    setY(r);
  }
  else
    return Object2D::setValue(name, value, args);

  return true;
}

}
