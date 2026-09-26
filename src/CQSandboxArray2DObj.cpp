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
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  auto *app = canvas()->app();
//auto *tcl = canvas()->tcl();

  if      (name == "value") {
    if (args.size() != 1)
      return app->errorMsg("Invalid number of args for int_array get value");

    int i;
    if (! Util::stringToInt(args[0], i))
      return app->errorMsg("Invalid index for int_array get value");

    if (i < 0 || i >= int(values_.size()))
      return app->errorMsg("Invalid index for int_array get value");

    value = values_[i];
  }
  else if (name == "dim") {
    value = int(values_.size());
  }
  else if (name == "dup") {
    auto *obj = new IntArray2DObj(canvas(), values_.size());

    obj->values_ = values_;

    auto name = canvas()->addNewObject(obj);

    value = name;
  }
  else
    return Object2D::getValue(name, args, value);

  return true;
}

bool
IntArray2DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas()->app();
//auto *tcl = canvas()->tcl();

  if (name == "value") {
    if (args.size() != 1)
      return app->errorMsg("Invalid number of args for int_array set value");

    int i;
    if (! Util::stringToInt(value, i))
      return app->errorMsg("Invalid index for int_array set value");

    if (i >= int(values_.size()))
      return app->errorMsg("Invalid index for int_array set value");

    int v;
    if (! Util::stringToInt(args[0], v))
      return app->errorMsg("Invalid int_array value");

    values_[i] = v;
  }
  else
    return Object2D::setValue(name, value, args);

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
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  auto *app = canvas()->app();
//auto *tcl = canvas()->tcl();

  if      (name == "value") {
    if (args.size() != 1)
      return app->errorMsg("Invalid number of args for real_array get value");

    int i;
    if (! Util::stringToInt(args[0], i))
      return app->errorMsg("Invalid index for real_array get value");

    if (i < 0 || i >= int(values_.size()))
      return app->errorMsg("Invalid index for real_array get value");

    value = values_[i];
  }
  else if (name == "dim") {
    value = int(values_.size());
  }
  else if (name == "dup") {
    auto *obj = new RealArray2DObj(canvas(), values_.size());

    obj->values_ = values_;

    auto name = canvas()->addNewObject(obj);

    value = name;
  }
  else
    return Object2D::getValue(name, args, value);

  return true;
}

bool
RealArray2DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas()->app();
//auto *tcl = canvas()->tcl();

  if (name == "value") {
    if (args.size() != 1)
      return app->errorMsg("Invalid number of args for real_array set value");

    int i;
    if (! Util::stringToInt(value, i))
      return app->errorMsg("Invalid index for real_array set value");

    if (i >= int(values_.size()))
      return app->errorMsg("Invalid index for real_array set value");

    double v;
    if (! Util::stringToReal(args[0], v))
      return app->errorMsg("Invalid real_array value");

    values_[i] = v;
  }
  else
    return Object2D::setValue(name, value, args);

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
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if      (name == "value") {
    if (args.size() != 1 && args.size() != 2)
      return app->errorMsg("Invalid number of args for real_matrix get value");

    int i1, i2;
    if (args.size() == 2) {
      if (! Util::stringToInt(args[0], i1) || ! Util::stringToInt(args[1], i2))
        return app->errorMsg("Invalid indices for real_matrix get value");

      if (i1 < 0 || i1 >= int(values_.size()) || i2 < 0 || i2 >= int(values_[0].size()))
        return app->errorMsg("Invalid indices for real_matrix get value");
    }
    else {
      std::vector<int> i;
      if (! Util::stringToIntArray(tcl, args[0], i) || i.size() != 2)
        return app->errorMsg("Invalid indices for real_matrix get value");

      i1 = i[0];
      i2 = i[1];
    }

    value = values_[i1][i2];
  }
  else if (name == "dim0") {
    value = int(values_.size());
  }
  else if (name == "dim1") {
    value = int(values_[0].size());
  }
  else if (name == "dup") {
    auto *obj = new IntMatrix2DObj(canvas(), values_.size(), values_[0].size());

    obj->values_ = values_;

    auto name = canvas()->addNewObject(obj);

    value = name;
  }
  else
    return Object2D::getValue(name, args, value);

  return true;
}

bool
IntMatrix2DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if (name == "value") {
    if (args.size() != 1 && args.size() != 2)
      return app->errorMsg("Invalid number of args for int_matrix get value");

    int v;

    int i1, i2;
    if (args.size() == 2) {
      if (! Util::stringToInt(value, i1) || ! Util::stringToInt(args[0], i2))
        return app->errorMsg("Invalid indices for int_matrix get value");

      if (i1 < 0 || i1 >= int(values_.size()) || i2 < 0 || i2 >= int(values_[0].size()))
        return app->errorMsg("Invalid indices for int_matrix get value");

      //---

      if (! Util::stringToInt(args[1], v))
        return app->errorMsg("Invalid int_matrix value");
    }
    else {
      std::vector<int> i;
      if (! Util::stringToIntArray(tcl, value, i) || i.size() != 2)
        return app->errorMsg("Invalid indices for int_matrix get value");

      i1 = i[0];
      i2 = i[1];

      //---

      if (! Util::stringToInt(args[0], v))
        return app->errorMsg("Invalid int_matrix value");
    }

    values_[i1][i2] = v;
  }
  else
    return Object2D::setValue(name, value, args);

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
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if      (name == "value") {
    if (args.size() != 1 && args.size() != 2)
      return app->errorMsg("Invalid number of args for real_matrix get value");

    int i1, i2;
    if (args.size() == 2) {
      if (! Util::stringToInt(args[0], i1) || ! Util::stringToInt(args[1], i2))
        return app->errorMsg("Invalid indices for real_matrix get value");

      if (i1 < 0 || i1 >= int(values_.size()) || i2 < 0 || i2 >= int(values_[0].size()))
        return app->errorMsg("Invalid indices for real_matrix get value");
    }
    else {
      std::vector<int> i;
      if (! Util::stringToIntArray(tcl, args[0], i) || i.size() != 2)
        return app->errorMsg("Invalid indices for real_matrix get value");

      i1 = i[0];
      i2 = i[1];
    }

    value = values_[i1][i2];
  }
  else if (name == "dim0") {
    value = int(values_.size());
  }
  else if (name == "dim1") {
    value = int(values_[0].size());
  }
  else if (name == "dup") {
    auto *obj = new RealMatrix2DObj(canvas(), values_.size(), values_[0].size());

    obj->values_ = values_;

    auto name = canvas()->addNewObject(obj);

    value = name;
  }
  else
    return Object2D::getValue(name, args, value);

  return true;
}

bool
RealMatrix2DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas()->app();
  auto *tcl = canvas()->tcl();

  if (name == "value") {
    if (args.size() != 1 && args.size() != 2)
      return app->errorMsg("Invalid number of args for real_matrix get value");

    double v;

    int i1, i2;
    if (args.size() == 2) {
      if (! Util::stringToInt(value, i1) || ! Util::stringToInt(args[0], i2))
        return app->errorMsg("Invalid indices for real_matrix get value");

      if (i1 < 0 || i1 >= int(values_.size()) || i2 < 0 || i2 >= int(values_[0].size()))
        return app->errorMsg("Invalid indices for real_matrix get value");

      if (! Util::stringToReal(args[1], v))
        return app->errorMsg("Invalid real_array value");
    }
    else {
      std::vector<int> i;
      if (! Util::stringToIntArray(tcl, value, i) || i.size() != 2)
        return app->errorMsg("Invalid indices for real_matrix get value");

      i1 = i[0];
      i2 = i[1];

      if (! Util::stringToReal(args[0], v))
        return app->errorMsg("Invalid real_array value");
    }

    values_[i1][i2] = v;
  }
  else
    return Object2D::setValue(name, value, args);

  return true;
}

}
