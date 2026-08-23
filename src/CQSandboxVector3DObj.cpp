#include <CQSandboxVector3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

Object3D *
Vector3DObj::
create(Canvas3D *canvas, const QStringList &args)
{
  if (args.size() > 1)
    return nullptr;

  auto *tcl = canvas->app()->tcl();

  auto *obj = new Vector3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

Vector3DObj::
Vector3DObj(Canvas3D *canvas) :
 Object3D(canvas, Type::VECTOR)
{
}

void
Vector3DObj::
init()
{
  Object3D::init();
}

bool
Vector3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if      (name == "x")
    value = v_.x();
  else if (name == "y")
    value = v_.y();
  else
    return Object3D::getValue(name, args, value);

  return true;
}

bool
Vector3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "x")
    v_.setX(Util::stringToReal(value));
  else if (name == "y")
    v_.setY(Util::stringToReal(value));
  else
    return Object3D::setValue(name, value, args);

  return true;
}

}
