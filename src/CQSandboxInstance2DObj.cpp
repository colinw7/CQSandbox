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
  auto *app = canvas->app();

  if (objc < 2)
    return app->errorMsg("Invalid number of args for instance create");

  auto *tcl = canvas->tcl();

  // 0 is command name (skip)

  auto className = tcl->qstringFromObj(objv[1]);

  auto *classObj = canvas->getClass(className);
  if (! classObj)
    return app->errorMsg("No class of name '" + className + "'");

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
  auto *app = canvas_->app();

  auto methodName   = QString("init");
  auto instanceName = getCommandName();

  Tcl_Obj *res;
  if (! classObj_->invokeMethod(methodName, instanceName, args_, res))
    return app->errorMsg("Failed to invoke method '" + methodName + "'");

  return true;
}

QString
Instance2DObj::
getCommandName() const
{
  return QString("sb2d::I_%1.%2").arg(classObj_->name()).arg(ind_);
}

bool
Instance2DObj::
getTclValue(const QString &name, const TclObjs &args, Tcl_Obj* &res)
{
  auto *app = canvas_->app();

  if (args.size() != 0)
    return app->errorMsg("Invalid number of args for instance get");

  auto pn = nameValue_.find(name);
  if (pn == nameValue_.end())
    return app->errorMsg("No value of name '" + name + "' in class");

  res = (*pn).second;

  Tcl_IncrRefCount(res);

  return true;
}

bool
Instance2DObj::
setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &args)
{
  auto *app = canvas_->app();

  if (args.size() != 0)
    return app->errorMsg("Invalid number of args for instance set");

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

bool
Instance2DObj::
execTcl(const QString &op, const TclObjs &objs, Tcl_Obj* &res)
{
  if (! classObj_->invokeMethod(op, getCommandName(), objs, res))
    return false;

  return true;
}

}
