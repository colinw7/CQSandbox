#include <CQSandboxVector3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

Object3D *
Vector2Obj3D::
create(Canvas3D *canvas, const QStringList &args)
{
  auto *tcl = canvas->tcl();

  auto *obj = new Vector2Obj3D(canvas);

  if      (args.size() == 0) {
  }
  else if (args.size() == 1) {
    QStringList strs;
    if (! tcl->splitList(args[0], strs))
      return nullptr;

    if (strs.size() != 2)
      return nullptr;

    double x, y;
    if (! Util::stringToReal(strs[0], x) || ! Util::stringToReal(strs[1], y))
      return nullptr;

    obj->setVector(CVector2D(x, y));
  }
  else if (args.size() == 2) {
    double x, y;
    if (! Util::stringToReal(args[0], x) || ! Util::stringToReal(args[1], y))
      return nullptr;

    obj->setVector(CVector2D(x, y));
  }
  else
    return nullptr;

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

Vector2Obj3D::
Vector2Obj3D(Canvas3D *canvas) :
 Object3D(canvas, Type::VECTOR2)
{
}

void
Vector2Obj3D::
init()
{
  Object3D::init();
}

bool
Vector2Obj3D::
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
Vector2Obj3D::
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

//---

Object3D *
Vector3Obj3D::
create(Canvas3D *canvas, const QStringList &args)
{
  auto *tcl = canvas->tcl();

  auto *obj = new Vector3Obj3D(canvas);

  if      (args.size() == 0) {
  }
  else if (args.size() == 1) {
    QStringList strs;
    if (! tcl->splitList(args[0], strs))
      return nullptr;

    if (strs.size() != 3)
      return nullptr;

    double x, y, z;
    if (! Util::stringToReal(strs[0], x) ||
        ! Util::stringToReal(strs[1], y) ||
        ! Util::stringToReal(strs[2], z))
      return nullptr;

    obj->setVector(CVector3D(x, y, z));
  }
  else if (args.size() == 3) {
    double x, y, z;
    if (! Util::stringToReal(args[0], x) ||
        ! Util::stringToReal(args[1], y) ||
        ! Util::stringToReal(args[2], z))
      return nullptr;

    obj->setVector(CVector3D(x, y, z));
  }
  else
    return nullptr;

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

Vector3Obj3D::
Vector3Obj3D(Canvas3D *canvas) :
 Object3D(canvas, Type::VECTOR3)
{
}

void
Vector3Obj3D::
init()
{
  Object3D::init();
}

bool
Vector3Obj3D::
getTclValue(const QString &name, const TclObjs &args, Tcl_Obj* &res)
{
  auto *tcl = canvas()->tcl();

  if      (name == "x")
    res = tcl->newRealObj(v_.x());
  else if (name == "y")
    res = tcl->newRealObj(v_.y());
  else if (name == "z")
    res = tcl->newRealObj(v_.z());
  else if (name == "string")
    res = tcl->newStringObj(QString("%1 %2 %3").arg(v_.x()).arg(v_.y()).arg(v_.z()));
  else
    return Object3D::getTclValue(name, args, res);

  return true;
}

bool
Vector3Obj3D::
setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &args)
{
  auto *tcl = canvas()->tcl();

  if      (name == "x") {
    double x;
    if (! tcl->getRealFromObj(value, x))
      return false;
    v_.setX(x);
  }
  else if (name == "y") {
    double y;
    if (! tcl->getRealFromObj(value, y))
      return false;
    v_.setY(y);
  }
  else if (name == "z") {
    double z;
    if (! tcl->getRealFromObj(value, z))
      return false;
    v_.setZ(z);
  }
  else
    return Object3D::setTclValue(name, value, args);

  return true;
}

bool
Vector3Obj3D::
execTcl(const QString &op, const TclObjs &objs, Tcl_Obj* &res)
{
  auto *tcl = canvas()->tcl();

  auto newVector = [&](const CVector3D &v) {
    auto *obj = new Vector3Obj3D(canvas());

    auto name = canvas()->addNewObject(obj);

    obj->v_ = v;

    obj->init();

    return tcl->newStringObj(name);
  };

  auto vectorArg = [&](int i=0) {
    QString name;
    if (! tcl->getStringFromObj(objs[i], name))
      return static_cast<Vector3Obj3D *>(nullptr);

    auto *v = dynamic_cast<Vector3Obj3D *>(canvas()->getObjectByName(name));

    return v;
  };

  if      (op == "normalize") {
    auto v1 = v_.normalized();

    res = newVector(v1);
  }
  else if (op == "length") {
    auto l = v_.length();

    res = tcl->newRealObj(l);
  }
  else if (op == "dot") {
    if (objs.size() != 1)
      return false;

    auto *v = vectorArg();
    if (! v) return false;

    auto r = v_.dotProduct(v->v_);

    res = tcl->newRealObj(r);
  }
  else if (op == "cross") {
    if (objs.size() != 1)
      return false;

    auto *v = vectorArg();
    if (! v) return false;

    auto v1 = v_.crossProduct(v->v_);

    res = newVector(v1);
  }
  else if (op == "distance") {
    if (objs.size() != 1)
      return false;

    auto *v = vectorArg();
    if (! v) return false;

    auto d = v_.getDistance(v->v_);

    res = tcl->newRealObj(d);
  }
  else if (op == "cmp") {
    if (objs.size() != 1)
      return false;

    QString name;
    if (! tcl->getStringFromObj(objs[0], name))
      return false;

    auto *v = dynamic_cast<Vector3Obj3D *>(canvas()->getObjectByName(name));
    if (! v)
      return false;

    auto i = v_.cmp(v->v_);

    res = tcl->newIntObj(i);
  }
  else if (op == "+" || op == "add") {
    if (objs.size() != 1)
      return false;

    auto *v = vectorArg();
    if (! v) return false;

    auto v1 = v_ + v->v_;

    res = newVector(v1);
  }
  else if (op == "-" || op == "subtract") {
    if (objs.size() != 1)
      return false;

    auto *v = vectorArg();
    if (! v) return false;

    auto v1 = v_ - v->v_;

    res = newVector(v1);
  }
  else if (op == "scale") {
   if (objs.size() != 1)
      return false;

    double s;
    if (! tcl->getRealFromObj(objs[0], s))
      return false;

    auto v1 = s*v_;

    res = newVector(v1);
  }
  else if (op == "*" || op == "multiply") {
    if (objs.size() != 1)
      return false;

    auto *v = vectorArg();
    if (! v) return false;

    auto v1 = v_ * v->v_;

    res = newVector(v1);
  }
  else if (op == "print") {
    std::cerr << v_ << "\n";
  }
  else
    return Object3D::execTcl(op, objs, res);

  return true;
}

}
