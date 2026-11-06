#include <CQSandboxAnimPoint3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

Object3D *
AnimPoint3DObj::
create(Canvas3D *canvas, const QStringList &args)
{
  auto *tcl = canvas->tcl();

  auto *obj = new AnimPoint3DObj(canvas);

  if (args.size() >= 2) {
    CPoint3D p;
    if (Util::stringToPoint3D(tcl, args[0], p))
      obj->point_.setValue(p);
  }

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

AnimPoint3DObj::
AnimPoint3DObj(Canvas3D *canvas) :
 Object3D(canvas, Type::ANIM_POINT)
{
}

void
AnimPoint3DObj::
init()
{
  Object3D::init();
}

bool
AnimPoint3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if      (name == "value") {
    value = Util::point3DToString(point_.value());
  }
  else if (name == "target") {
    value = Util::point3DToString(point_.target());
  }
  else if (name == "steps") {
    value = int(point_.steps());
  }
  else if (name == "can_step") {
    value = point_.canStep();
  }
  else if (name == "step") {
    value = int(point_.getStep());
  }
  else if (name == "style") {
    if      (point_.style() == AnimatePoint3D::Style::ONE_SHOT)
      value = "one_shot";
    else if (point_.style() == AnimatePoint3D::Style::BOUNCE_ONCE)
      value = "bounce_once";
    else if (point_.style() == AnimatePoint3D::Style::BOUNCE_ALWAYS)
      value = "bounce_always";
    else
      return false;
  }
  else
    return Object3D::getValue(name, args, value);

  return true;
}

bool
AnimPoint3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas_->tcl();

  if      (name == "value") {
    CPoint3D p;
    if (! Util::stringToPoint3D(tcl, value, p))
      return false;

    point_.setValue(p);
  }
  else if (name == "target") {
    CPoint3D p;
    if (! Util::stringToPoint3D(tcl, value, p))
      return false;

    point_.setTarget(p);
  }
  else if (name == "steps") {
    int i;
    if (! Util::stringToInt(value, i))
      return false;

    point_.setSteps(i);
  }
  else if (name == "style") {
    auto lstr = value.toLower();

    if      (lstr == "one_shot")
      point_.setStyle(AnimatePoint3D::Style::ONE_SHOT);
    else if (lstr == "bounce_once")
      point_.setStyle(AnimatePoint3D::Style::BOUNCE_ONCE);
    else if (lstr == "bounce_always")
      point_.setStyle(AnimatePoint3D::Style::BOUNCE_ALWAYS);
    else
      return false;
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

bool
AnimPoint3DObj::
exec(const QString &op, const QStringList &args, QVariant &res)
{
  if      (op == "step") {
    point_.step();
  }
  else if (op == "reset") {
    point_.reset();
  }
  else
    return Object3D::exec(op, args, res);

  return true;
}

}
