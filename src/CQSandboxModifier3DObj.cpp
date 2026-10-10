#include <CQSandboxModifier3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQGLUtil.h>
#include <CQTclUtil.h>

namespace CQSandbox {

Object3D *
Modifier3DObj::
create(Canvas3D *canvas, const QStringList &args)
{
  auto *tcl = canvas->tcl();

  Object3D *object { nullptr };

  if (args.size() > 0) {
    auto name = args[0];

    object = canvas->getObjectByName(name);

    if (! object)
      return nullptr;
  }

  auto *obj = new Modifier3DObj(canvas);

  if (object)
    obj->setObject(object);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

Modifier3DObj::
Modifier3DObj(Canvas3D *canvas) :
 Object3D(canvas, Type::MODIFIER)
{
}

void
Modifier3DObj::
init()
{
  Object3D::init();
}

void
Modifier3DObj::
setObject(Object3D *obj)
{
  if (object_)
    object_->removeModifier(this);

  object_ = obj;

  if (object_)
    object_->addModifier(this);
}

bool
Modifier3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
//auto *tcl = canvas()->tcl();

  if      (name == "type") {
    value = "";
  }
  else if (name == "object") {
    value = (object_ ? object_->getCommandName() : "");
  }
  else
    return Object3D::getValue(name, args, value);

  return true;
}

bool
Modifier3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->tcl();

  if      (name == "type") {
    auto lstr = value.toLower();

    if      (lstr == "array") {
      type_ = ModifierType::ARRAY;
    }
    else if (lstr == "mirror") {
      type_ = ModifierType::MIRROR;
    }
    else
      return false;
  }
  else if (name == "object") {
    auto *object = canvas()->getObjectByName(value);

    if (! object)
      return false;

    object_ = object;
  }
  else if (name == "count") {
    int n;
    if (! Util::stringToInt(value, n))
      return false;

    arrayData_.count = n;
  }
  else if (name == "offset") {
    CPoint3D p;
    if (! Util::stringToPoint3D(tcl, value, p))
      return false;

    arrayData_.offset = p;
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
Modifier3DObj::
drawData(ShaderProgram *program, CQGLBuffer *buffer,
         const CQGLBuffer::DrawData &data, const CMatrix3DH &modelMatrix)
{
  if      (type_ == ModifierType::ARRAY) {
    auto matrix = modelMatrix;

    for (int i = 1; i < arrayData_.count; ++i) {
      matrix.translated(arrayData_.offset.x, arrayData_.offset.y, arrayData_.offset.z);

      program->setUniformValue("model", CQGLUtil::toQMatrix(matrix));

      buffer->drawData(data);
    }
  }
  else if (type_ == ModifierType::MIRROR) {
  }
}

}
