#include <CQSandboxVector2DObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

bool
Vector2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  auto *tcl = canvas->tcl();

  auto *obj = new Vector2DObj(canvas);

  if (args.size() >= 2) {
    double x, y;
    if (! Util::stringToReal(args[0], x) || ! Util::stringToReal(args[1], y))
      return false;

    obj->setX(x);
    obj->setY(y);
  }

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

Vector2DObj::
Vector2DObj(Canvas2D *canvas) :
 Object2D(canvas, Type::VECTOR)
{
}

bool
Vector2DObj::
getTclValue(const QString &name, const TclObjs &objs, Tcl_Obj* &res)
{
  auto *tcl = canvas()->tcl();

  if      (name == "x")
    res = tcl->newRealObj(v_.x());
  else if (name == "y")
    res = tcl->newRealObj(v_.y());
  else
    return Object2D::getTclValue(name, objs, res);

  return true;
}

bool
Vector2DObj::
setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &objs)
{
  auto *tcl = canvas()->tcl();

  if      (name == "x") {
    double r;
    if (! tcl->getRealFromObj(value, r))
      return false;

    setX(r);
  }
  else if (name == "y") {
    double r;
    if (! tcl->getRealFromObj(value, r))
      return false;

    setY(r);
  }
  else
    return Object2D::setTclValue(name, value, objs);

  return true;
}

bool
Vector2DObj::
execTcl(const QString &op, const TclObjs &objs, Tcl_Obj* &res)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if      (op == "inc.x") {
    if (objs.size() != 1)
      return false;

    double r;
    if (! tcl->getRealFromObj(objs[0], r))
      return false;

    setX(v_.x() + r);
  }
  else if (op == "inc.y") {
    if (objs.size() != 1)
      return false;

    double r;
    if (! tcl->getRealFromObj(objs[0], r))
      return false;

    setY(v_.y() + r);
  }
  else if (op == "dec.x") {
    if (objs.size() != 1)
      return false;

    double r;
    if (! tcl->getRealFromObj(objs[0], r))
      return false;

    setX(v_.x() - r);
  }
  else if (op == "dec.y") {
    if (objs.size() != 1)
      return false;

    double r;
    if (! tcl->getRealFromObj(objs[0], r))
      return false;

    setY(v_.y() - r);
  }
  else if (op == "add") {
    if (objs.size() != 1)
      return false;

    auto name = tcl->qstringFromObj(objs[0]);

    auto *vectorObj = dynamic_cast<Vector2DObj *>(canvas()->getObjectByName(name));
    if (! vectorObj)
      return app->errorMsg(QString("Failed to find vector '%1'").arg(name));

    v_ += vectorObj->v_;
  }
  else
    return Object2D::execTcl(op, objs, res);

  return true;
}

}
