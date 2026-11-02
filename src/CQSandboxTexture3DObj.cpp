#include <CQSandboxTexture3DObj.h>
#include <CQSandboxTexture.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

Object3D *
Texture3DObj::
create(Canvas3D *canvas, const QStringList &args)
{
  if (args.size() != 0)
    return nullptr;

  auto *tcl = canvas->tcl();

  auto *obj = new Texture3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  obj->setTexture(canvas->createTexture());

  tcl->setResult(name);

  return obj;
}

Texture3DObj::
Texture3DObj(Canvas3D *canvas) :
 Object3D(canvas, Type::TEXTURE)
{
}

void
Texture3DObj::
init()
{
  Object3D::init();
}

bool
Texture3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  return Object3D::getValue(name, args, value);
}

bool
Texture3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  return Object3D::setValue(name, value, args);
}

}
