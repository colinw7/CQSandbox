#ifndef CQSandboxVectorObj_H
#define CQSandboxVectorObj_H

#include <CQSandboxObject.h>

#include <CVector2D.h>

namespace CQSandbox {

class VectorObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  VectorObj(Canvas *canvas);

  const char *typeName() const override { return "vector"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 protected:
  CVector2D v_;
};

}

#endif
