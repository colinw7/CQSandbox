#include <CQSandboxShlib3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>

#define OS_UNIX 1
#include <CShLib.h>

#include <CFile.h>

namespace CQSandbox {

Object3D *
Shlib3DObj::
create(Canvas3D *canvas, const QStringList &args)
{
  if (args.size() < 1)
    return nullptr;

  auto libName = args[0];

  auto *tcl = canvas->tcl();

  auto *obj = new Shlib3DObj(canvas, libName);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

Shlib3DObj::
Shlib3DObj(Canvas3D *canvas, const QString &libName) :
 Object3D(canvas, Type::SHLIB), libName_(libName)
{
  auto *app = canvas_->app();

  auto moduleDirs = canvas_->moduleDirs();

  for (const auto &dir : moduleDirs) {
    auto libPath = dir + "/lib" + libName_ + ".so";

    if (! CFile::exists(libPath.toStdString()))
      continue;

    // open shared library
    shlib_ = new CShLib(libPath.toStdString());

    if (! shlib_->open()) {
      (void) app->errorMsg("Failed to Open Library '" + libName_ + "'");
      return;
    }

    libPath_ = libPath;

    break;
  }

  if (! shlib_)
    return;

  // get symbols
  auto getProc = [&](const QString &procName) {
    CShLibProc proc;

    if (! shlib_->getProc(procName.toStdString(), &proc)) {
      app->errorMsg("Failed to Find Symbol '" + procName + "'");
      return static_cast<void *>(nullptr);
    }

    return reinterpret_cast<void *>(proc);
  };

  initProc_ = getProc(libName_ + "_init");
  getProc_  = getProc(libName_ + "_get_value");
  setProc_  = getProc(libName_ + "_set_value");
  execProc_ = getProc(libName_ + "_exec");
}

Shlib3DObj::
~Shlib3DObj()
{
  delete shlib_;
}

void
Shlib3DObj::
init()
{
  Object3D::init();

  using InitProc = int (*)();

  if (initProc_)
    (void) (*reinterpret_cast<InitProc>(initProc_))();
}

bool
Shlib3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  auto *tcl = canvas()->tcl();

  using GetProc = int (*)(Tcl_Interp *interp, const char *name, Tcl_Obj **res);

  auto name1 = name.toStdString();

  if (getProc_) {
    Tcl_Obj* res { nullptr };

    if ((*reinterpret_cast<GetProc>(getProc_))(tcl->interp(), name1.c_str(), &res)) {
      value = tcl->variantFromObj(res);
      return true;
    }
  }

  return Object3D::getValue(name, args, value);
}

bool
Shlib3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->tcl();

  using SetProc = int (*)(Tcl_Interp *interp, const char *name, const char *value);

  auto name1  = name .toStdString();
  auto value1 = value.toStdString();

  if (setProc_) {
    if ((*reinterpret_cast<SetProc>(setProc_))(tcl->interp(), name1.c_str(), value1.c_str()))
      return true;
  }

  return Object3D::setValue(name, value, args);
}

bool
Shlib3DObj::
exec(const QString &op, const QStringList &args, QVariant &res)
{
  auto *tcl = canvas()->tcl();

  using ExecProc = int (*)(Tcl_Interp *interp, const char *op);

  auto op1 = op.toStdString();

  if (execProc_) {
    if ((*reinterpret_cast<ExecProc>(execProc_))(tcl->interp(), op1.c_str()))
      return true;
  }

  return Object3D::exec(op, args, res);
}

}
