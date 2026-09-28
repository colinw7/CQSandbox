#include <CQSandboxArray2DObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

bool
IntArray2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  auto *app = canvas->app();

  if (args.size() != 1)
    return app->errorMsg("Invalid number of args for int_array create");

  auto *tcl = canvas->tcl();

  int dim;
  if (! Util::stringToInt(args[0], dim))
    return app->errorMsg("Invalid dim for int_array create");

  if (dim <= 0)
    return app->errorMsg("Invalid dimension for int_array");

  auto *obj = new IntArray2DObj(canvas, dim);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

IntArray2DObj::
IntArray2DObj(Canvas2D *canvas, uint dim) :
 Object2D(canvas, Type::INT_ARRAY)
{
  values_.resize(dim);
}

bool
IntArray2DObj::
getTclValue(const QString &name, const TclObjs &objs, Tcl_Obj* &res)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if      (name == "value") {
    if (objs.size() != 1)
      return app->errorMsg("Invalid number of args for int_array get value");

    int i;
    if (! tcl->getIntFromObj(objs[0], i))
      return app->errorMsg("Invalid index for int_array get value");

    if (i < 0 || i >= int(values_.size()))
      return app->errorMsg("Invalid index for int_array get value");

    res = tcl->newIntObj(values_[i]);
  }
  else if (name == "dim") {
    res = tcl->newIntObj(values_.size());
  }
  else if (name == "dup") {
    auto *obj = new IntArray2DObj(canvas(), values_.size());

    obj->values_ = values_;

    auto name = canvas()->addNewObject(obj);

    res = tcl->newStringObj(name);
  }
  else
    return Object2D::getTclValue(name, objs, res);

  return true;
}

bool
IntArray2DObj::
setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &objs)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if (name == "value") {
    if (objs.size() != 1)
      return app->errorMsg("Invalid number of args for int_array set value");

    int i;
    if (! tcl->getIntFromObj(value, i))
      return app->errorMsg("Invalid index for int_array set value");

    if (i >= int(values_.size()))
      return app->errorMsg("Invalid index for int_array set value");

    int v;
    if (! tcl->getIntFromObj(objs[0], v))
      return app->errorMsg("Invalid int_array value");

    values_[i] = v;
  }
  else
    return Object2D::setTclValue(name, value, objs);

  return true;
}

//---

bool
RealArray2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  auto *app = canvas->app();

  if (args.size() != 1)
    return app->errorMsg("Invalid number of args for real_array create");

  auto *tcl = canvas->tcl();

  int dim;
  if (! Util::stringToInt(args[0], dim))
    return app->errorMsg("Invalid dim for real_array create");

  if (dim <= 0)
    return app->errorMsg("Invalid dimension for real_array");

  auto *obj = new RealArray2DObj(canvas, dim);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

RealArray2DObj::
RealArray2DObj(Canvas2D *canvas, uint dim) :
 Object2D(canvas, Type::REAL_ARRAY)
{
  values_.resize(dim);
}

bool
RealArray2DObj::
getTclValue(const QString &name, const TclObjs &objs, Tcl_Obj* &res)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if      (name == "value") {
    if (objs.size() != 1)
      return app->errorMsg("Invalid number of args for real_array get value");

    int i;
    if (! tcl->getIntFromObj(objs[0], i))
      return app->errorMsg("Invalid index for real_array get value");

    if (i < 0 || i >= int(values_.size()))
      return app->errorMsg("Invalid index for real_array get value");

    res = tcl->newRealObj(values_[i]);
  }
  else if (name == "dim") {
    res = tcl->newIntObj(values_.size());
  }
  else if (name == "dup") {
    auto *obj = new RealArray2DObj(canvas(), values_.size());

    obj->values_ = values_;

    auto name = canvas()->addNewObject(obj);

    res = tcl->newStringObj(name);
  }
  else
    return Object2D::getTclValue(name, objs, res);

  return true;
}

bool
RealArray2DObj::
setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &objs)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if (name == "value") {
    if (objs.size() != 1)
      return app->errorMsg("Invalid number of args for real_array set value");

    int i;
    if (! tcl->getIntFromObj(value, i))
      return app->errorMsg("Invalid index for real_array set value");

    if (i >= int(values_.size()))
      return app->errorMsg("Invalid index for real_array set value");

    double v;
    if (! tcl->getRealFromObj(objs[0], v))
      return app->errorMsg("Invalid real_array value");

    values_[i] = v;
  }
  else
    return Object2D::setTclValue(name, value, objs);

  return true;
}

//---

bool
IntMatrix2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  auto *app = canvas->app();

  if (args.size() != 2)
    return app->errorMsg("Invalid number of args for int_matrix create");

  auto *tcl = canvas->tcl();

  int dim1, dim2;
  if (! Util::stringToInt(args[0], dim1) || ! Util::stringToInt(args[1], dim2))
    return app->errorMsg("Invalid dim for int_matrix create");

  if (dim1 <= 0 || dim2 <= 0)
    return app->errorMsg("Invalid dimensions for int_matrix");

  auto *obj = new IntMatrix2DObj(canvas, dim1, dim2);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

IntMatrix2DObj::
IntMatrix2DObj(Canvas2D *canvas, uint dim1, uint dim2) :
 Object2D(canvas, Type::INT_MATRIX)
{
  values_.resize(dim1);

  for (auto &value : values_)
    value.resize(dim2);
}

bool
IntMatrix2DObj::
getTclValue(const QString &name, const TclObjs &objs, Tcl_Obj* &res)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if      (name == "value") {
    if (objs.size() != 1 && objs.size() != 2)
      return app->errorMsg("Invalid number of args for int_matrix get value");

    int i1, i2;
    if (objs.size() == 2) {
      if (! tcl->getIntFromObj(objs[0], i1) || ! tcl->getIntFromObj(objs[1], i2))
        return app->errorMsg("Invalid indices for int_matrix get value");

      if (i1 < 0 || i1 >= int(values_.size()) || i2 < 0 || i2 >= int(values_[0].size()))
        return app->errorMsg("Invalid indices for int_matrix get value");
    }
    else {
      int n = tcl->getObjLength(objs[0]);
      if (n != 2)
        return app->errorMsg("Invalid indices for int_matrix get value");

      if (! tcl->getIntFromObj(tcl->getListObj(objs[0], 0), i1) ||
          ! tcl->getIntFromObj(tcl->getListObj(objs[0], 1), i2))
        return app->errorMsg("Invalid indices for int_matrix get value");
    }

    res = tcl->newIntObj(values_[i1][i2]);
  }
  else if (name == "dim0") {
    res = tcl->newIntObj(values_.size());
  }
  else if (name == "dim1") {
    res = tcl->newIntObj(values_[0].size());
  }
  else if (name == "dup") {
    auto *obj = new IntMatrix2DObj(canvas(), values_.size(), values_[0].size());

    obj->values_ = values_;

    auto name = canvas()->addNewObject(obj);

    res = tcl->newStringObj(name);
  }
  else
    return Object2D::getTclValue(name, objs, res);

  return true;
}

bool
IntMatrix2DObj::
setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &objs)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if (name == "value") {
    if (objs.size() != 1 && objs.size() != 2)
      return app->errorMsg("Invalid number of args for int_matrix get value");

    int v;

    int i1, i2;
    if (objs.size() == 2) {
      if (! tcl->getIntFromObj(value, i1) || ! tcl->getIntFromObj(objs[0], i2))
        return app->errorMsg("Invalid indices for int_matrix get value");

      if (i1 < 0 || i1 >= int(values_.size()) || i2 < 0 || i2 >= int(values_[0].size()))
        return app->errorMsg("Invalid indices for int_matrix get value");

      //---

      if (! tcl->getIntFromObj(objs[1], v))
        return app->errorMsg("Invalid int_matrix value");
    }
    else {
      int n = tcl->getObjLength(objs[0]);
      if (n != 2)
        return app->errorMsg("Invalid indices for int_matrix get value");

      if (! tcl->getIntFromObj(tcl->getListObj(value, 0), i1) ||
          ! tcl->getIntFromObj(tcl->getListObj(value, 1), i2))
        return app->errorMsg("Invalid indices for int_matrix get value");

      //---

      if (! tcl->getIntFromObj(objs[0], v))
        return app->errorMsg("Invalid int_matrix value");
    }

    values_[i1][i2] = v;
  }
  else
    return Object2D::setTclValue(name, value, objs);

  return true;
}

//---

bool
RealMatrix2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  auto *app = canvas->app();

  if (args.size() != 2)
    return app->errorMsg("Invalid number of args for real_matrix create");

  auto *tcl = canvas->tcl();

  int dim1, dim2;
  if (! Util::stringToInt(args[0], dim1) || ! Util::stringToInt(args[1], dim2))
    return app->errorMsg("Invalid dim for real_matrix create");

  if (dim1 <= 0 || dim2 <= 0)
    return app->errorMsg("Invalid dimensions for real_matrix");

  auto *obj = new RealMatrix2DObj(canvas, dim1, dim2);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

RealMatrix2DObj::
RealMatrix2DObj(Canvas2D *canvas, uint dim1, uint dim2) :
 Object2D(canvas, Type::REAL_MATRIX)
{
  values_.resize(dim1);

  for (auto &value : values_)
    value.resize(dim2);
}

bool
RealMatrix2DObj::
getTclValue(const QString &name, const TclObjs &objs, Tcl_Obj* &res)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if      (name == "value") {
    if (objs.size() != 1 && objs.size() != 2)
      return app->errorMsg("Invalid number of args for real_matrix get value");

    int i1, i2;
    if (objs.size() == 2) {
      if (! tcl->getIntFromObj(objs[0], i1) || ! tcl->getIntFromObj(objs[1], i2))
        return app->errorMsg("Invalid indices for real_matrix get value");

      if (i1 < 0 || i1 >= int(values_.size()) || i2 < 0 || i2 >= int(values_[0].size()))
        return app->errorMsg("Invalid indices for real_matrix get value");
    }
    else {
      int n = tcl->getObjLength(objs[0]);
      if (n != 2)
        return app->errorMsg("Invalid indices for real_matrix get value");

      if (! tcl->getIntFromObj(tcl->getListObj(objs[0], 0), i1) ||
          ! tcl->getIntFromObj(tcl->getListObj(objs[0], 1), i2))
        return app->errorMsg("Invalid indices for real_matrix get value");
    }

    res = tcl->newRealObj(values_[i1][i2]);
  }
  else if (name == "dim0") {
    res = tcl->newIntObj(values_.size());
  }
  else if (name == "dim1") {
    res = tcl->newIntObj(values_[0].size());
  }
  else if (name == "dup") {
    auto *obj = new RealMatrix2DObj(canvas(), values_.size(), values_[0].size());

    obj->values_ = values_;

    auto name = canvas()->addNewObject(obj);

    res = tcl->newStringObj(name);
  }
  else
    return Object2D::getTclValue(name, objs, res);

  return true;
}

bool
RealMatrix2DObj::
setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &objs)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if (name == "value") {
    if (objs.size() != 1 && objs.size() != 2)
      return app->errorMsg("Invalid number of args for real_matrix get value");

    double v;

    int i1, i2;
    if (objs.size() == 2) {
      if (! tcl->getIntFromObj(value, i1) || ! tcl->getIntFromObj(objs[0], i2))
        return app->errorMsg("Invalid indices for real_matrix get value");

      if (i1 < 0 || i1 >= int(values_.size()) || i2 < 0 || i2 >= int(values_[0].size()))
        return app->errorMsg("Invalid indices for real_matrix get value");

      //---

      if (! tcl->getRealFromObj(objs[1], v))
        return app->errorMsg("Invalid real_matrix value");
    }
    else {
      int n = tcl->getObjLength(objs[0]);
      if (n != 2)
        return app->errorMsg("Invalid indices for real_matrix get value");

      if (! tcl->getIntFromObj(tcl->getListObj(value, 0), i1) ||
          ! tcl->getIntFromObj(tcl->getListObj(value, 1), i2))
        return app->errorMsg("Invalid indices for real_matrix get value");

      //---

      if (! tcl->getRealFromObj(objs[0], v))
        return app->errorMsg("Invalid real_matrix value");
    }

    values_[i1][i2] = v;
  }
  else
    return Object2D::setTclValue(name, value, objs);

  return true;
}

}
