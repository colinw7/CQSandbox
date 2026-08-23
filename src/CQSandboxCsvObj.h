#ifndef CQSandboxCsvObj_H
#define CQSandboxCsvObj_H

#include <CQSandboxObject.h>

class CQCsvModel;

namespace CQSandbox {

class CsvObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  CsvObj(Canvas *canvas, const QString &filename);

  const char *typeName() const override { return "csv"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  bool exec(const QString &op, const QStringList &args, QVariant &res) override;

 protected:
  QString filename_;

  CQCsvModel *csv_ { nullptr };
};

}

#endif
