#include <CQSandboxClass2DObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQSandboxClass2DUtil.h>

#include <CQTclUtil.h>
#include <CQUtil.h>

namespace CQSandbox {

bool
Class2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  auto *app = canvas->app();
  auto *tcl = canvas->tcl();

  if (args.size() != 1)
    return app->errorMsg("Invalid number of args for class create");

  auto name = args[0];

  if (canvas->hasClass(name))
    return app->errorMsg("Class already exists");

  auto *obj = new Class2DObj(canvas, name);

  auto objName = canvas->addClass(obj);

  if (! obj->init())
    return false;

  tcl->setResult(objName);

  return true;
}

Class2DObj::
Class2DObj(Canvas2D *canvas, const QString &name) :
 Object2D(canvas, Type::CLASS), name_(name)
{
}

QString
Class2DObj::
getCommandName() const
{
  return QString("sb2d::C_%1.%2").arg(name()).arg(ind_);
}

bool
Class2DObj::
addMethod(const QString &name, const QString &args, const QString &body)
{
  auto *app = canvas_->app();
  auto *tcl = canvas_->tcl();

  QStringList args1;
  if (! tcl->splitList(args, args1))
    return app->errorMsg("Invalid method arg list '" + args + "'");

  methods_[name] = MethodData(args1, body);

  return true;
}

bool
Class2DObj::
invokeMethod(const QString &methodName, const QString &instanceName,
             const std::vector<Tcl_Obj *> &args, Tcl_Obj* &res)
{
  auto *app = canvas_->app();

  auto pm = methods_.find(methodName);

  if (pm == methods_.end())
    return app->errorMsg("No method of name '" + methodName + "'");

  auto &methodData = (*pm).second;

  if (methodData.args.size() != int(args.size() + 1))
    return app->errorMsg("Method argument mismatch for '" + methodName + "'");

  auto *tcl = canvas_->tcl();

  bool rc = true;

  auto instanceName1 = instanceName.toStdString();

  auto *instanceNameObj = Tcl_NewStringObj(instanceName1.c_str(), instanceName1.size());

#if 1
  // create proc for method body
  if (! methodData.bodySet) {
    methodData.bodySet  = true;
    methodData.procName = name_ + "_" + methodName;

    auto procCmd = "proc " + methodData.procName + " {";

    methodData.callCmd = methodData.procName;

    for (int i = 0; i < methodData.args.size(); ++i) {
      procCmd += " _" + methodData.args[i];

      methodData.callCmd += " $_" + methodData.args[i];
    }

    procCmd += " } {";

    for (int i = 0; i < methodData.args.size(); ++i) {
      procCmd += "\n  set " + methodData.args[i] + " $_" + methodData.args[i];
    }

    procCmd += methodData.body;
    procCmd += "}";

    CQTcl::EvalData evalData;
    evalData.showError = true;

    // std::cerr << procCmd << "\n";
    if (! tcl->eval(procCmd, evalData))
      return false;

    // std::cerr << methodData.callCmd << "\n";
  }
#endif

#if 0
  // set tcl variable for body
  for (int i = 0; i < methodData.args.size(); ++i) {
    auto varName = methodData.args[i].toStdString();

    auto *varNameObj = Tcl_NewStringObj(varName.c_str(), varName.size());
    Tcl_IncrRefCount(varNameObj);

    Tcl_Obj *valueObj;

    if (i == 0)
      valueObj = instanceNameObj;
    else
      valueObj = args[i - 1];

    Tcl_IncrRefCount(valueObj);

    Tcl_ObjSetVar2(tcl->interp(), varNameObj, nullptr, valueObj, 0);

    Tcl_DecrRefCount(valueObj);
    Tcl_DecrRefCount(varNameObj);
  }

  CQTcl::EvalData evalData;
  evalData.showError = true;

  if (! tcl->eval(methodData.body, evalData))
    return false;

  res = evalData.res;
#endif

#if 1
  // set variables for proc call
  for (int i = 0; i < methodData.args.size(); ++i) {
    auto varName = "_" + methodData.args[i].toStdString();

    auto *varNameObj = Tcl_NewStringObj(varName.c_str(), varName.size());
    Tcl_IncrRefCount(varNameObj);

    Tcl_Obj *valueObj;

    if (i == 0)
      valueObj = instanceNameObj;
    else
      valueObj = args[i - 1];

    Tcl_IncrRefCount(valueObj);

    Tcl_ObjSetVar2(tcl->interp(), varNameObj, nullptr, valueObj, 0);

    Tcl_DecrRefCount(valueObj);
    Tcl_DecrRefCount(varNameObj);
  }
#endif

#if 0
  if (! tcl->eval(methodData.callCmd, evalData))
    return false;

  res = evalData.res;
#else
  auto callCmd = methodData.callCmd.toStdString();

  auto rc1 = Tcl_EvalEx(tcl->interp(), callCmd.c_str(), -1, 0);

  if (rc1 != TCL_OK) {
    auto errMsg = tcl->errorInfo(rc1);

    if (errMsg == "")
      errMsg = "'" + QString::fromStdString(callCmd) + "' Failed";

    std::cerr << errMsg << "\n";

    return false;
  }

  res = Tcl_GetObjResult(tcl->interp());
#endif

#if 0
  std::vector<std::string> argNames;
  std::vector<Tcl_Obj *>   argValues;

  argNames .push_back(methodData.args[0].toStdString());
  argValues.push_back(instanceNameObj);

  for (int i = 1; i < methodData.args.size(); ++i) {
    argNames .push_back(methodData.args[i].toStdString());
    argValues.push_back(args[i - 1]);
  }

  Tcl_Obj *res1;
  if (Tcl_InvokeMethod(tcl->interp(), argNames, argValues,
                       methodData.body.toStdString(), &res1) != TCL_OK)
    rc = false;

  if (rc) {
    auto *res2 = Tcl_DuplicateObj(res1);

    res = tcl->variantFromObj(res1);

    Tcl_SetObjResult(tcl->interp(), res2);
  }
#endif

  return rc;
}

bool
Class2DObj::
getValue(const QString &varName, const QStringList &args, QVariant &value)
{
  return Object2D::getValue(varName, args, value);
}

bool
Class2DObj::
setValue(const QString &varName, const QString &value, const QStringList &args)
{
  return Object2D::setValue(varName, value, args);
}

}
