#include <CQSandboxMaterial3DObj.h>
#include <CQSandboxMaterial3D.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

Object3D *
Material3DObj::
create(Canvas3D *canvas, const QStringList &args)
{
  if (args.size() != 0)
    return nullptr;

  auto *tcl = canvas->tcl();

  auto *obj = new Material3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  obj->setMaterial(canvas->createMaterial());

  tcl->setResult(name);

  return obj;
}

Material3DObj::
Material3DObj(Canvas3D *canvas) :
 Object3D(canvas, Type::MATERIAL)
{
}

void
Material3DObj::
init()
{
  Object3D::init();
}

bool
Material3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if (name == "diffuse") {
    CRGBA c;

    if (material_)
      c = material_->getDiffuse();

    value = Util::RGBAToQColor(c);
  }
  else
    return Object3D::getValue(name, args, value);

  return true;
}

bool
Material3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  return Object3D::setValue(name, value, args);
}

}
