#ifndef CQSandboxCsv3DObj_H
#define CQSandboxCsv3DObj_H

#include <CQSandboxObject3D.h>

class CQCsvModel;

namespace CQSandbox {

class Csv3DObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Csv3DObj(Canvas3D *canvas, const QString &filename);

  const char *typeName() const override { return "Csv"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  QVariant exec(const QString &op, const QStringList &args) override;

  //---

  void init() override;

 protected:
  QString filename_;

  CQCsvModel *csv_ { nullptr };
};

}

#endif
