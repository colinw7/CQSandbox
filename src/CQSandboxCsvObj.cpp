#include <CQSandboxCsvObj.h>
#include <CQSandboxCanvas.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQCsvModel.h>
#include <CQTclUtil.h>

namespace CQSandbox {

bool
CsvObj::
create(Canvas *canvas, const QStringList &args)
{
  if (args.size() != 1)
    return false;

  auto *tcl = canvas->app()->tcl();

  auto filename = args[0];

  auto *obj = new CsvObj(canvas, filename);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

CsvObj::
CsvObj(Canvas *canvas, const QString &filename) :
 Object(canvas), filename_(filename)
{
  csv_ = new CQCsvModel;
}

bool
CsvObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  auto *app = canvas()->app();

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
    return Object::getValue(name, args, value);

  return true;
}

bool
CsvObj::
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
    return Object::setValue(name, value, args);

  return true;
}

bool
CsvObj::
exec(const QString &op, const QStringList &args, QVariant &res)
{
  if (op == "load") {
    if (! csv_->load(filename_))
      return false;

    return true;
  }
  else
    return Object::exec(op, args, res);
}

}
