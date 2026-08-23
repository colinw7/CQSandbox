#include <CQSandboxArrayObj.h>
#include <CQSandboxCanvas.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

bool
ArrayObj::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() != 2)
    return false;

  auto *tcl = canvas->app()->tcl();

  auto dim0 = Util::stringToInt(args[0]);
  auto dim1 = Util::stringToInt(args[1]);

  auto *obj = new ArrayObj(canvas, dim0, dim1);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

ArrayObj::
ArrayObj(Canvas *canvas, uint dim0, uint dim1) :
 Object(canvas), a_(dim0, dim1, 0.0)
{
}

ArrayObj::
ArrayObj(Canvas *canvas, const CArray2D<double> &a) :
 Object(canvas), a_(a)
{
}

bool
ArrayObj::
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
    auto *obj = new ArrayObj(canvas(), a_);

    auto name = canvas()->addNewObject(obj);

    value = name;
  }
  else
    return Object::getValue(name, args, value);

  return true;
}

bool
ArrayObj::
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
    return Object::setValue(name, value, args);

  return true;
}

}
