#include <CQSandboxCsv3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQCsvModel.h>
#include <CQTclUtil.h>

namespace CQSandbox {

Object3D *
Csv3DObj::
create(Canvas3D *canvas, const QStringList &args)
{
  if (args.size() != 1)
    return nullptr;

  auto *tcl = canvas->app()->tcl();

  auto filename = args[0];

  auto *obj = new Csv3DObj(canvas, filename);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

Csv3DObj::
Csv3DObj(Canvas3D *canvas, const QString &filename) :
 Object3D(canvas, Type::CSV), filename_(filename)
{
  csv_ = new CQCsvModel;
}

void
Csv3DObj::
init()
{
  Object3D::init();
}

bool
Csv3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  auto *app = canvas_->app();

  if      (name == "filename")
    value = filename_;
  else if (name == "comment_header")
    value = csv_->isCommentHeader();
  else if (name == "first_line_header")
    value = csv_->isFirstLineHeader();
  else if (name == "first_column_header")
    value = csv_->isFirstColumnHeader();
  else if (name == "num_rows")
    value = csv_->rowCount();
  else if (name == "num_columns" || name == "num_cols")
    value = csv_->columnCount();
  else if (name == "data") {
    if (args.size() == 2) {
      auto row = Util::stringToInt(args[0]);
      auto col = Util::stringToInt(args[1]);

      auto ind = csv_->index(row, col, QModelIndex());

      value = csv_->data(ind);
    }
    else
      return app->errorMsg("missing row/col for data");
  }
  else
    return Object3D::getValue(name, args, value);

  return true;
}

bool
Csv3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "filename")
    filename_ = value;
  else if (name == "comment_header")
    csv_->setCommentHeader(Util::stringToBool(value));
  else if (name == "first_line_header")
    csv_->setFirstLineHeader(Util::stringToBool(value));
  else if (name == "first_column_header")
    csv_->setFirstColumnHeader(Util::stringToBool(value));
  else
    return Object3D::setValue(name, value, args);

  return true;
}

bool
Csv3DObj::
exec(const QString &op, const QStringList &args, QVariant &res)
{
  if (op == "load") {
    bool b = csv_->load(filename_);

    res = QVariant(b);
  }
  else
    return Object3D::exec(op, args, res);

  return true;
}

}
