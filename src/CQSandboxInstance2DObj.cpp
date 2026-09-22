#include <CQSandboxInstance2DObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxClass2DObj.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

bool
Instance2DObj::
create(Canvas2D *canvas, int objc, const Tcl_Obj **objv)
{
  if (objc < 2)
    return false;

  auto *tcl = canvas->tcl();

  // 0 is command name (skip)

  auto className = tcl->qstringFromObj(objv[1]);

  auto *classObj = canvas->getClass(className);
  if (! classObj) return false;

  std::vector<Tcl_Obj *> args;
  for (int i = 2; i < objc; ++i)
    args.push_back(const_cast<Tcl_Obj *>(objv[i]));

  auto *obj = new Instance2DObj(canvas, classObj, args);

  auto objName = canvas->addNewObject(obj);

  if (! obj->init())
    return false;

  tcl->setResult(objName);

  return true;
}

Instance2DObj::
Instance2DObj(Canvas2D *canvas, Class2DObj *classObj, const std::vector<Tcl_Obj *> &args) :
 Object2D(canvas, Type::INSTANCE), classObj_(classObj)
{
  for (auto *arg : args) {
    Tcl_IncrRefCount(arg);

    args_.push_back(arg);
  }
}

bool
Instance2DObj::
init()
{
  auto methodName   = QString("init");
  auto instanceName = getCommandName();

  QVariant res;
  if (! classObj_->invokeMethod(methodName, instanceName, args_, res))
    return false;

  return true;
}

bool
Instance2DObj::
getTclValue(const QString &name, const TclObjs &args, Tcl_Obj* &res)
{
  if (args.size() != 0)
    return false;

  auto pn = nameValue_.find(name);
  if (pn == nameValue_.end()) return false;

  res = (*pn).second;

  Tcl_IncrRefCount(res);

  return true;
}

bool
Instance2DObj::
setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &args)
{
  if (args.size() != 0)
    return false;

  auto pn = nameValue_.find(name);

  if (pn != nameValue_.end()) {
    auto *obj = (*pn).second;

    Tcl_DecrRefCount(obj);

    (*pn).second = value;
  }
  else
    nameValue_[name] = value;

  Tcl_IncrRefCount(value);

  return true;
}

}
