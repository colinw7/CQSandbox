#include <CQSandboxVectorObj.h>
#include <CQSandboxCanvas.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

bool
VectorObj::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() > 1)
    return false;

  auto *tcl = canvas->app()->tcl();

  auto *obj = new VectorObj(canvas);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

VectorObj::
VectorObj(Canvas *canvas) :
 Object(canvas)
{
}

bool
VectorObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if      (name == "x")
    value = v_.x();
  else if (name == "y")
    value = v_.y();
  else
    return Object::getValue(name, args, value);

  return true;
}

bool
VectorObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "x")
    v_.setX(Util::stringToReal(value));
  else if (name == "y")
    v_.setY(Util::stringToReal(value));
  else
    return Object::setValue(name, value, args);

  return true;
}

}
