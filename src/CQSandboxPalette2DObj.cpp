#include <CQSandboxPalette2DObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

bool
Palette2DObj::
create(Canvas2D *canvas, const QStringList &)
{
  auto *tcl = canvas->tcl();

  auto *obj = new Palette2DObj(canvas);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

Palette2DObj::
Palette2DObj(Canvas2D *canvas) :
 Object2D(canvas, Type::PALETTE)
{
}

bool
Palette2DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  //auto *tcl = canvas()->tcl();

  if (name == "interp") {
    if (args.size() < 1)
      return false;

    double r;
    if (! Util::stringToReal(args[0], r))
      return false;

    auto c = colorRange_.interpColor(r);

    value = Util::RGBAToQColor(c);
  }
  else
    return Object2D::getValue(name, args, value);

  return true;
}

bool
Palette2DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if (name == "mode") {
    if      (value == "rgb_range")
      colorRange_.setType(CColorRange::Type::RGB_RANGE);
    else if (value == "moreland")
      colorRange_.setType(CColorRange::Type::MORELAND);
    else if (value == "plasma")
      colorRange_.setType(CColorRange::Type::PLASMA);
    else
      return false;
  }
  else
    return Object2D::setValue(name, value, args);

  return true;
}

}
