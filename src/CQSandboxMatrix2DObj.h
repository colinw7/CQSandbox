#ifndef CQSandboxMatrix2DObj_H
#define CQSandboxMatrix2DObj_H

#include <CQSandboxObject2D.h>

#include <CArray2D.h>

namespace CQSandbox {

class RealMatrix2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  RealMatrix2DObj(Canvas2D *canvas, uint dim0, uint dim1);
  RealMatrix2DObj(Canvas2D *canvas, const CArray2D<double> &a);

  const char *typeName() const override { return "matrix"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 protected:
  CArray2D<double> a_;
};

}

#endif
