#include <CQSandboxJson3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>

namespace CQSandbox {

Object3D *
Json3DObj::
create(Canvas3D *canvas, const QStringList &args)
{
  if (args.size() != 1)
    return nullptr;

  auto *tcl = canvas->tcl();

  auto filename = args[0];

  auto *obj = new Json3DObj(canvas, filename);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

Json3DObj::
Json3DObj(Canvas3D *canvas, const QString &filename) :
 Object3D(canvas, Type::CSV), filename_(filename)
{
  json_ = new CJson;
}

Json3DObj::
~Json3DObj()
{
  delete json_;
}

void
Json3DObj::
init()
{
  Object3D::init();
}

bool
Json3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if (! json_)
    return false;

  //auto *app = canvas_->app();

  auto indToValue = [&](const QString &ind, CJson::ValueP &value) {
    bool ok;
    auto i = ind.toInt(&ok);
    if (! ok) return false;

    value = json_->indToValue(i);
    return true;
  };

  auto value1 = value_;

  if (args.size() >= 1) {
    if (! indToValue(args[0], value1))
      return false;
  }

  if      (name == "filename")
    value = filename_;
  else if (name == "to_string") {
    if (! value1)
      return false;

    value = QString::fromStdString(value1->to_string());
  }
  else if (name == "type") {
    if (! value1)
      return false;

    value = QString::fromStdString(CJson::typeName(value1->type()));
  }
  else if (name == "value") {
    if      (value1->isNumber())
      value = value1->toNumber();
    else if (value1->isString())
      value = QString::fromStdString(value1->toString());
  }
  else if (name == "num_values") {
    if (! value1->isComposite())
      return false;

    auto composite = value1->cast<CJson::Composite>();

    value = composite->numValues();
  }
  else if (name == "values") {
    if (! value1->isComposite())
      return false;

    auto composite = value1->cast<CJson::Composite>();

    auto n = composite->numValues();

    QVariantList vars;

    for (uint i = 0; i < n; ++i) {
      QStringList strs;

      strs.push_back(QString::fromStdString(composite->indexKey(i)));
      strs.push_back(QString::number(composite->indexValue(i)->ind()));

      vars.push_back(strs);
    }

    value = vars;
  }
  else
    return Object3D::getValue(name, args, value);

  return true;
}

bool
Json3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "filename")
    filename_ = value;
  else
    return Object3D::setValue(name, value, args);

  return true;
}

bool
Json3DObj::
exec(const QString &op, const QStringList &args, QVariant &res)
{
  if (op == "load") {
    bool b = json_->loadFile(filename_.toStdString(), value_);

    res = QVariant(b);
  }
  else
    return Object3D::exec(op, args, res);

  return true;
}

}
