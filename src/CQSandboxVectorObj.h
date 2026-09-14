#ifndef CQSandboxVectorObj_H
#define CQSandboxVectorObj_H

#include <CQSandboxObject2D.h>

#include <CVector2D.h>

namespace CQSandbox {

class VectorObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  VectorObj(Canvas2D *canvas);

  const char *typeName() const override { return "vector"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 protected:
  CVector2D v_;
};

}

#endif
