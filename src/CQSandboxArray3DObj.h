#ifndef CQSandboxArray3DObj_H
#define CQSandboxArray3DObj_H

#include <CQSandboxObject3D.h>

#include <CArray2D.h>

class CQCsvModel;

namespace CQSandbox {

class Array3DObj : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  Array3DObj(Canvas3D *canvas, uint dim0, uint dim1);
  Array3DObj(Canvas3D *canvas, const CArray2D<double> &a);

  const char *typeName() const override { return "array"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  //---

  void init() override;

 protected:
  CArray2D<double> a_;
};

}

#endif
