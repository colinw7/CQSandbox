#include <CQSandboxAnimReal3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

Object3D *
AnimReal3DObj::
create(Canvas3D *canvas, const QStringList &args)
{
  auto *tcl = canvas->tcl();

  auto *obj = new AnimReal3DObj(canvas);

  if (args.size() >= 2) {
    double r;
    if (Util::stringToReal(args[0], r))
      obj->real_.setValue(r);
  }

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

AnimReal3DObj::
AnimReal3DObj(Canvas3D *canvas) :
 Object3D(canvas, Type::ANIM_REAL)
{
}

void
AnimReal3DObj::
init()
{
  Object3D::init();
}

bool
AnimReal3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if      (name == "value") {
    value = real_.value();
  }
  else if (name == "target") {
    value = real_.target();
  }
  else if (name == "steps") {
    value = int(real_.steps());
  }
  else if (name == "can_step") {
    value = real_.canStep();
  }
  else if (name == "step") {
    value = int(real_.getStep());
  }
  else if (name == "style") {
    if      (real_.style() == AnimateReal::Style::ONE_SHOT)
      value = "one_shot";
    else if (real_.style() == AnimateReal::Style::BOUNCE_ONCE)
      value = "bounce_once";
    else if (real_.style() == AnimateReal::Style::BOUNCE_ALWAYS)
      value = "bounce_always";
    else
      return false;
  }
  else
    return Object3D::getValue(name, args, value);

  return true;
}

bool
AnimReal3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "value") {
    double r;
    if (! Util::stringToReal(value, r))
      return false;

    real_.setValue(r);
  }
  else if (name == "target") {
    double r;
    if (! Util::stringToReal(value, r))
      return false;

    real_.setTarget(r);
  }
  else if (name == "steps") {
    int i;
    if (! Util::stringToInt(value, i))
      return false;

    real_.setSteps(i);
  }
  else if (name == "style") {
    auto lstr = value.toLower();

    if      (lstr == "one_shot")
      real_.setStyle(AnimateReal::Style::ONE_SHOT);
    else if (lstr == "bounce_once")
      real_.setStyle(AnimateReal::Style::BOUNCE_ONCE);
    else if (lstr == "bounce_always")
      real_.setStyle(AnimateReal::Style::BOUNCE_ALWAYS);
    else
      return false;
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

bool
AnimReal3DObj::
exec(const QString &op, const QStringList &args, QVariant &res)
{
  if      (op == "step") {
    real_.step();
  }
  else if (op == "reset") {
    real_.reset();
  }
  else
    return Object3D::exec(op, args, res);

  return true;
}

}
