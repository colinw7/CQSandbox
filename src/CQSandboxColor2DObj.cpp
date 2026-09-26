#include <CQSandboxColor2DObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

bool
Color2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  auto *tcl = canvas->tcl();

  auto *obj = new Color2DObj(canvas);

  CRGBA c;

  if      (args.size() == 1) {
    if (! Util::stringToRGBA(tcl, args[0], c))
      return false;

    obj->setColor(c);
  }
  else if (args.size() == 3) {
    double r, g, b;
    if (! Util::stringToReal(args[0], r) ||
        ! Util::stringToReal(args[1], g) ||
        ! Util::stringToReal(args[2], b))
      return false;

    c = CRGBA(r, g, b);
  }
  else if (args.size() == 4) {
    double r, g, b, a;
    if (! Util::stringToReal(args[0], r) ||
        ! Util::stringToReal(args[1], g) ||
        ! Util::stringToReal(args[2], b) ||
        ! Util::stringToReal(args[3], a))
      return false;

    c = CRGBA(r, g, b, a);
  }
  else
    return false;

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

Color2DObj::
Color2DObj(Canvas2D *canvas) :
 Object2D(canvas, Type::COLOR)
{
}

bool
Color2DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if      (name == "r")
    value = c_.getRed();
  else if (name == "g")
    value = c_.getGreen();
  else if (name == "b")
    value = c_.getBlue();
  else if (name == "name")
    value = QString::fromStdString(c_.stringEncodeARGB());
  else
    return Object2D::getValue(name, args, value);

  return true;
}

bool
Color2DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "r") {
    double r;
    if (! Util::stringToReal(value, r))
      return false;

    c_.setRed(r);
  }
  else if (name == "g") {
    double r;
    if (! Util::stringToReal(value, r))
      return false;

    c_.setGreen(r);
  }
  else if (name == "b") {
    double r;
    if (! Util::stringToReal(value, r))
      return false;

    c_.setBlue(r);
  }
  else
    return Object2D::setValue(name, value, args);

  return true;
}

}
