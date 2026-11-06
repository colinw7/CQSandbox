#ifndef CQSandboxAnimPoint3DObj_H
#define CQSandboxAnimPoint3DObj_H

#include <CQSandboxObject3D.h>
#include <CQSandboxAnim.h>

class CQAnimPointModel;

namespace CQSandbox {

class AnimPoint3DObj : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  AnimPoint3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "anim_point"; }

  //---

  void loadBuffer() override { }

  void render() override { }

  //---

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  bool exec(const QString &op, const QStringList &args, QVariant &res) override;

  //---

  void init() override;

 private:
  AnimatePoint3D point_;
};

}

#endif
