#include <CQSandboxMatrix2DObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

bool
Matrix2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  if (args.size() != 2)
    return false;

  auto *tcl = canvas->tcl();

  auto dim0 = Util::stringToInt(args[0]);
  auto dim1 = Util::stringToInt(args[1]);

  auto *obj = new Matrix2DObj(canvas, dim0, dim1);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

Matrix2DObj::
Matrix2DObj(Canvas2D *canvas, uint dim0, uint dim1) :
 Object2D(canvas, Type::MATRIX), a_(dim0, dim1, 0.0)
{
}

Matrix2DObj::
Matrix2DObj(Canvas2D *canvas, const CArray2D<double> &a) :
 Object2D(canvas, Type::MATRIX), a_(a)
{
}

bool
Matrix2DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  auto *tcl = canvas()->tcl();

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
    auto *obj = new Matrix2DObj(canvas(), a_);

    auto name = canvas()->addNewObject(obj);

    value = name;
  }
  else
    return Object2D::getValue(name, args, value);

  return true;
}

bool
Matrix2DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->tcl();

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
    return Object2D::setValue(name, value, args);

  return true;
}

}
