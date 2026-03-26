#include <CQSandboxCsv3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQCsvModel.h>
#include <CQTclUtil.h>

namespace CQSandbox {

bool
Csv3DObj::
create(Canvas3D *canvas, const QStringList &args)
{
  if (args.size() != 1)
    return false;

  auto *tcl = canvas->app()->tcl();

  auto filename = args[0];

  auto *obj = new Csv3DObj(canvas, filename);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Csv3DObj::
Csv3DObj(Canvas3D *canvas, const QString &filename) :
 Object3D(canvas), filename_(filename)
{
  csv_ = new CQCsvModel;
}

void
Csv3DObj::
init()
{
  Object3D::init();
}

QVariant
Csv3DObj::
getValue(const QString &name, const QStringList &args)
{
  auto *app = canvas_->app();

  if      (name == "filename")
    return filename_;
  else if (name == "comment_header")
    return csv_->isCommentHeader();
  else if (name == "first_line_header")
    return csv_->isFirstLineHeader();
  else if (name == "first_column_header")
    return csv_->isFirstColumnHeader();
  else if (name == "num_rows")
    return csv_->rowCount();
  else if (name == "num_columns" || name == "num_cols")
    return csv_->columnCount();
  else if (name == "data") {
    if (args.size() == 2) {
      auto row = Util::stringToInt(args[0]);
      auto col = Util::stringToInt(args[1]);

      auto ind = csv_->index(row, col, QModelIndex());

      return csv_->data(ind);
    }
    else {
      app->errorMsg("missing row/col for data");
      return QVariant();
    }
  }
  else
    return Object3D::getValue(name, args);
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

QVariant
Csv3DObj::
exec(const QString &op, const QStringList &args)
{
  if (op == "load") {
    if (! csv_->load(filename_))
      return QVariant(0);

    return QVariant(1);
  }
  else
    return Object3D::exec(op, args);
}

}
