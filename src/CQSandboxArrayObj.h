#ifndef CQSandboxArrayObj_H
#define CQSandboxArrayObj_H

#include <CQSandboxObject.h>

#include <CArray2D.h>

namespace CQSandbox {

class ArrayObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  ArrayObj(Canvas *canvas, uint dim0, uint dim1);
  ArrayObj(Canvas *canvas, const CArray2D<double> &a);

  const char *typeName() const override { return "array"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 protected:
  CArray2D<double> a_;
};

}

#endif
