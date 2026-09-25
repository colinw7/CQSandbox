#include <CQSandboxArray2DObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

bool
ObjArray2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  auto *app = canvas->app();

  if (args.size() != 1)
    return app->errorMsg("Invalid number of args for obj_array create");

  auto *tcl = canvas->tcl();

  auto dim = Util::stringToInt(args[0]);

  if (dim <= 0)
    return app->errorMsg("Invalid dimension for obj_array");

  auto *obj = new ObjArray2DObj(canvas, dim);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

ObjArray2DObj::
ObjArray2DObj(Canvas2D *canvas, uint dim) :
 Object2D(canvas, Type::OBJ_ARRAY)
{
  values_.resize(dim);
}

bool
ObjArray2DObj::
getTclValue(const QString &name, const TclObjs &objs, Tcl_Obj* &res)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if      (name == "value") {
    if (objs.size() != 1)
      return app->errorMsg("Invalid number of args for obj_array get value");

    int i;
    if (! tcl->getIntFromObj(objs[0], i))
      return app->errorMsg("Invalid index for obj_array get value");

    if (i < 0 || i >= int(values_.size()))
      return app->errorMsg("Invalid index for obj_array get value");

    auto *value = values_[i];

    if (! value)
      return app->errorMsg("no value for obj_array");

    res = value;
  }
  else if (name == "has_value") {
    if (objs.size() != 1)
      return app->errorMsg("Invalid number of args for obj_array get has_value");

    int i;
    if (! tcl->getIntFromObj(objs[0], i))
      return app->errorMsg("Invalid index for obj_array get has_value");

    if (i < 0 || i >= int(values_.size()))
      return app->errorMsg("Invalid index for obj_array get has_value");

    auto *value = values_[i];

    res = tcl->newIntObj(value ? 1 : 0);
  }
  else if (name == "dim") {
    res = tcl->newIntObj(values_.size());
  }
  else
    return Object2D::getTclValue(name, objs, res);

  return true;
}

bool
ObjArray2DObj::
setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &objs)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if (name == "value") {
    if (objs.size() != 1)
      return app->errorMsg("Invalid number of args for obj_array set value");

    int i;
    if (! tcl->getIntFromObj(value, i))
      return app->errorMsg("Invalid index for obj_array set value");

    if (i >= int(values_.size()))
      return app->errorMsg("Invalid index for obj_array set value");

    values_[i] = objs[0];

    Tcl_IncrRefCount(values_[i]);
  }
  else
    return Object2D::setTclValue(name, value, objs);

  return true;
}

//---

bool
ObjMatrix2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  auto *app = canvas->app();

  if (args.size() != 2)
    return app->errorMsg("Invalid number of args for obj_matrix create");

  auto *tcl = canvas->tcl();

  auto dim1 = Util::stringToInt(args[0]);
  auto dim2 = Util::stringToInt(args[1]);

  if (dim1 <= 0 || dim2 <= 0)
    return app->errorMsg("Invalid dimensions for obj_matrix");

  auto *obj = new ObjMatrix2DObj(canvas, dim1, dim2);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

ObjMatrix2DObj::
ObjMatrix2DObj(Canvas2D *canvas, uint dim1, uint dim2) :
 Object2D(canvas, Type::OBJ_MATRIX)
{
  values_.resize(dim1);

  for (auto &value : values_)
    value.resize(dim2);
}

bool
ObjMatrix2DObj::
getTclValue(const QString &name, const TclObjs &objs, Tcl_Obj* &res)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if      (name == "value") {
    if (objs.size() != 2)
      return app->errorMsg("Invalid number of args for obj_matrix get value");

    int i1, i2;
    if (! tcl->getIntFromObj(objs[0], i1) || ! tcl->getIntFromObj(objs[1], i2))
      return app->errorMsg("Invalid indices for obj_matrix get value");

    if (i1 < 0 || i1 >= int(values_.size()) || i2 < 0 || i2 >= int(values_[0].size()))
      return app->errorMsg("Invalid indices for obj_matrix get value");

    auto *value = values_[i1][i2];

    if (! value)
      return app->errorMsg("no value for obj_matrix");

    res = value;
  }
  else if (name == "has_value") {
    if (objs.size() != 2)
      return app->errorMsg("Invalid number of args for obj_matrix get has_value");

    int i1, i2;
    if (! tcl->getIntFromObj(objs[0], i1) || ! tcl->getIntFromObj(objs[1], i2))
      return app->errorMsg("Invalid indices for obj_matrix get has_value");

    if (i1 < 0 || i1 >= int(values_.size()) || i2 < 0 || i2 >= int(values_[0].size()))
      return app->errorMsg("Invalid indices for obj_matrix get has_value");

    auto *value = values_[i1][i2];

    res = tcl->newIntObj(value ? 1 : 0);
  }
  else if (name == "dim1") {
    res = tcl->newIntObj(values_.size());
  }
  else if (name == "dim2") {
    res = tcl->newIntObj(values_[0].size());
  }
  else
    return Object2D::getTclValue(name, objs, res);

  return true;
}

bool
ObjMatrix2DObj::
setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &objs)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if (name == "value") {
    if (objs.size() != 2)
      return app->errorMsg("Invalid number of args for obj_matrix set value");

    int i1, i2;
    if (! tcl->getIntFromObj(value, i1) || ! tcl->getIntFromObj(objs[0], i2))
      return app->errorMsg("Invalid indices for obj_matrix set value");

    if (i1 < 0 || i1 >= int(values_.size()) || i2 < 0 || i2 >= int(values_[0].size()))
      return app->errorMsg("Invalid indices for obj_matrix set value");

    values_[i1][i2] = objs[1];

    Tcl_IncrRefCount(values_[i1][i2]);
  }
  else
    return Object2D::setTclValue(name, value, objs);

  return true;
}

}
