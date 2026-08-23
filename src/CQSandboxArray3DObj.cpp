#include <CQSandboxArray3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

Object3D *
Array3DObj::
create(Canvas3D *canvas, const QStringList &args)
{
  if (args.size() != 2)
    return nullptr;

  auto *tcl = canvas->app()->tcl();

  auto dim0 = Util::stringToInt(args[0]);
  auto dim1 = Util::stringToInt(args[1]);

  auto *obj = new Array3DObj(canvas, dim0, dim1);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

Array3DObj::
Array3DObj(Canvas3D *canvas, uint dim0, uint dim1) :
 Object3D(canvas, Type::ARRAY), a_(dim0, dim1, 0.0)
{
}

Array3DObj::
Array3DObj(Canvas3D *canvas, const CArray2D<double> &a) :
 Object3D(canvas, Type::ARRAY), a_(a)
{
}

void
Array3DObj::
init()
{
  Object3D::init();
}

bool
Array3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  auto *app = canvas_->app();
  auto *tcl = app->tcl();

  if      (name == "value") {
    uint dim0, dim1;

    if      (args.size() == 2) {
      dim0 = Util::stringToInt(args[0]);
      dim1 = Util::stringToInt(args[1]);
    }
    else if (args.size() == 1) {
      std::vector<int> a;
      if (! Util::stringToIntArray(tcl, args[0], a) || a.size() != 2)
        return false;

      dim0 = a[0];
      dim1 = a[1];
    }
    else
      return false;

    if (! a_.validIndex(dim0, dim1))
      return false;

    value = a_.get(dim0, dim1);
  }
  else if (name == "dim0") {
    value = int(a_.dim(0));
  }
  else if (name == "dim1") {
    value = int(a_.dim(1));
  }
  else if (name == "dup") {
    auto *obj = new Array3DObj(canvas(), a_);

    auto name = canvas()->addNewObject(obj);

    value = name;
  }
  else
    return Object3D::getValue(name, args, value);

  return true;
}

bool
Array3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas()->app();
  auto *tcl = app->tcl();

  if (name == "value") {
    uint dim0, dim1;

    if      (args.size() == 2) {
      dim0 = Util::stringToInt(args[0]);
      dim1 = Util::stringToInt(args[1]);
    }
    else if (args.size() == 1) {
      std::vector<int> a;
      if (! Util::stringToIntArray(tcl, args[0], a) || a.size() != 2)
        return false;

      dim0 = a[0];
      dim1 = a[1];
    }
    else
      return false;

    if (! a_.validIndex(dim0, dim1))
      return false;

    auto r = Util::stringToReal(value);

    a_.set(dim0, dim1, r);
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

}
