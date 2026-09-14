#include <CQSandboxShlib3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxShlib.h>

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

  if (initProc_)
    (void) (*reinterpret_cast<CQSandboxShLib::InitProc>(initProc_))();
}

bool
Shlib3DObj::
getTclValue(const QString &name, const TclObjs &args, Tcl_Obj* &res)
{
  if (getProc_) {
    auto *tcl = canvas()->tcl();

    auto name1 = name.toStdString();

    if ((*reinterpret_cast<CQSandboxShLib::GetProc>(getProc_))(
          tcl->interp(), name1.c_str(), args.size(), const_cast<Tcl_Obj **>(&args[0]), &res))
      return true;
  }

  return Object3D::getTclValue(name, args, res);
}

bool
Shlib3DObj::
setTclValue(const QString &name, const QString &value, const TclObjs &args)
{
  if (setProc_) {
    auto *tcl = canvas()->tcl();

    auto name1  = name .toStdString();
    auto value1 = value.toStdString();

    if ((*reinterpret_cast<CQSandboxShLib::SetProc>(setProc_))(
          tcl->interp(), name1.c_str(), value1.c_str(),
          args.size(), const_cast<Tcl_Obj **>(&args[0])))
      return true;
  }

  return Object3D::setTclValue(name, value, args);
}

bool
Shlib3DObj::
execTcl(const QString &op, const TclObjs &args, Tcl_Obj* &res)
{
  if (execProc_) {
    auto *tcl = canvas()->tcl();

    auto op1 = op.toStdString();

    if ((*reinterpret_cast<CQSandboxShLib::ExecProc>(execProc_))(
          tcl->interp(), op1.c_str(), args.size(), const_cast<Tcl_Obj **>(&args[0]), &res))
      return true;
  }

  return Object3D::execTcl(op, args, res);
}

}
