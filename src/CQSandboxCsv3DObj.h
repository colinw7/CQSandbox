#ifndef CQSandboxCsv3DObj_H
#define CQSandboxCsv3DObj_H

#include <CQSandboxObject3D.h>

class CQCsvModel;

namespace CQSandbox {

class Csv3DObj : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  Csv3DObj(Canvas3D *canvas, const QString &filename);

  const char *typeName() const override { return "csv"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  bool exec(const QString &op, const QStringList &args, QVariant &res) override;

  //---

  void init() override;

  CBBox3D calcBBox() override { return bbox_; }

 protected:
  QString filename_;

  CQCsvModel *csv_ { nullptr };
};

}

#endif
