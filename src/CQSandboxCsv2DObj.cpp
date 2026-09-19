#include <CQSandboxCsv2DObj.h>
#include <CQSandboxCanvas2D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQCsvModel.h>
#include <CQTclUtil.h>

namespace CQSandbox {

bool
Csv2DObj::
create(Canvas2D *canvas, const QStringList &args)
{
  if (args.size() != 1)
    return false;

  auto *tcl = canvas->tcl();

  auto filename = args[0];

  auto *obj = new Csv2DObj(canvas, filename);

  auto name = canvas->addNewObject(obj);

  tcl->setResult(name);

  return true;
}

Csv2DObj::
Csv2DObj(Canvas2D *canvas, const QString &filename) :
 Object2D(canvas, Type::CSV), filename_(filename)
{
  csv_ = new CQCsvModel;
}

bool
Csv2DObj::
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
    return Object2D::getValue(name, args, value);

  return true;
}

bool
Csv2DObj::
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
    return Object2D::setValue(name, value, args);

  return true;
}

bool
Csv2DObj::
exec(const QString &op, const QStringList &args, QVariant &res)
{
  if (op == "load") {
    bool b = csv_->load(filename_);

    res = QVariant(b);
  }
  else
    return Object2D::exec(op, args, res);

  return true;
}

}
