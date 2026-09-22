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
  if (args.size() != 1)
    return false;

  auto name = args[0];

  auto *obj = new Class2DObj(canvas, name);

  canvas->addClass(obj);

  return true;
}

Class2DObj::
Class2DObj(Canvas2D *canvas, const QString &name) :
 Object2D(canvas, Type::CLASS), name_(name)
{
}

bool
Class2DObj::
addMethod(const QString &name, const QString &args, const QString &body)
{
  auto *tcl = canvas_->tcl();

  QStringList args1;
  if (! tcl->splitList(args, args1))
    return false;

  methods_[name] = MethodData(args1, body);

  return true;
}

bool
Class2DObj::
invokeMethod(const QString &name, const QString &instanceName,
             const std::vector<Tcl_Obj *> &args, QVariant &res)
{
  auto pm = methods_.find(name);
  if (pm == methods_.end()) return false;

  const auto &methodData = (*pm).second;

  if (methodData.args.size() != int(args.size() + 1))
    return false;

  auto *tcl = canvas_->tcl();

  bool rc = true;

#if 1
  for (int i = 0; i < methodData.args.size(); ++i) {
    auto varName = methodData.args[i].toStdString();

    auto *varNameObj = Tcl_NewStringObj(varName.c_str(), varName.size());

    Tcl_IncrRefCount(varNameObj);

    Tcl_Obj *valueObj;

    if (i == 0) {
      auto instanceName1 = instanceName.toStdString();

      valueObj = Tcl_NewStringObj(instanceName1.c_str(), instanceName1.size());
    }
    else {
      valueObj = args[i - 1];
    }

    Tcl_IncrRefCount(valueObj);

    Tcl_ObjSetVar2(tcl->interp(), varNameObj, nullptr, valueObj, 0);

    Tcl_DecrRefCount(valueObj);
    Tcl_DecrRefCount(varNameObj);
  }

  CQTcl::EvalData evalData;
  if (! tcl->eval(methodData.body, evalData))
    return false;

  res = evalData.res;
#else
  std::vector<std::string> argNames;
  std::vector<Tcl_Obj *>   argValues;

  argNames .push_back(methodData.args[0].toStdString());
  argValues.push_back(instanceName.toStdString());

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
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  return Object2D::getValue(name, args, value);
}

bool
Class2DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  return Object2D::setValue(name, value, args);
}

}
