#ifndef CQSandboxCsv2DObj_H
#define CQSandboxCsv2DObj_H

#include <CQSandboxObject2D.h>

class CQCsvModel;

namespace CQSandbox {

class Csv2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  Csv2DObj(Canvas2D *canvas, const QString &filename);

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
