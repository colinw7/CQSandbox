#include <CQSandboxArray2DObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

bool
Array2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  if (args.size() != 1)
    return false;

  auto *tcl = canvas->tcl();

  auto dim = Util::stringToInt(args[0]);

  auto *obj = new Array2DObj(canvas, dim);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

Array2DObj::
Array2DObj(Canvas2D *canvas, uint dim) :
 Object2D(canvas, Type::ARRAY)
{
  values_.resize(dim);
}

bool
Array2DObj::
getTclValue(const QString &name, const TclObjs &objs, Tcl_Obj* &res)
{
  auto *tcl = canvas()->tcl();

  if      (name == "value") {
    if (objs.size() != 1)
      return false;

    int i;
    if (! tcl->getIntFromObj(objs[0], i))
      return false;

    if (i < 0 || i >= int(values_.size()))
      return false;

    if (! values_[i])
      return false;

    res = values_[i];
  }
  else if (name == "dim") {
    res = tcl->newIntObj(values_.size());
  }
  else
    return Object2D::getTclValue(name, objs, res);

  return true;
}

bool
Array2DObj::
setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &objs)
{
  auto *tcl = canvas()->tcl();

  if (name == "value") {
    if (objs.size() != 1)
      return false;

    int i;
    if (! tcl->getIntFromObj(value, i))
      return false;

    if (i >= int(values_.size()))
      return false;

    values_[i] = objs[0];

    Tcl_IncrRefCount(values_[i]);
  }
  else
    return Object2D::setTclValue(name, value, objs);

  return true;
}

}
