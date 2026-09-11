#ifndef CQSandboxAnimReal3DObj_H
#define CQSandboxAnimReal3DObj_H

#include <CQSandboxObject3D.h>
#include <CQSandboxAnim.h>

class CQAnimRealModel;

namespace CQSandbox {

class AnimReal3DObj : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  AnimReal3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "anim_real"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  bool exec(const QString &op, const QStringList &args, QVariant &res) override;

  //---

  void init() override;

 private:
  AnimateReal real_;
};

}

#endif
