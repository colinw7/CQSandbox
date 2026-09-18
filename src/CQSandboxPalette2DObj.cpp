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

  if      (name == "interp") {
    if (args.size() < 1)
      return false;

    double r;
    if (! Util::stringToReal(args[0], r))
      return false;

    auto c = colorRange_.interpColor(r);

    value = Util::RGBAToQColor(c);
  }
  else if (name == "palette_names") {
    auto names = colorRange_.typeNames();

    QStringList names1;
    for (const auto &name : names)
      names1 << QString::fromStdString(name);

    value = names1;
  }
  else if (name == "name_type") {
    if (args.size() < 1)
      return false;

    auto type = colorRange_.nameToType(args[0].toStdString());

    value = int(type) - 1;
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
    auto type = colorRange_.nameToType(value.toStdString());

    if (type != CColorRange::Type::NONE)
      colorRange_.setType(type);
    else
      return false;
  }
  else
    return Object2D::setValue(name, value, args);

  return true;
}

}
