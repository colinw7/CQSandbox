#ifndef CQSandboxShlibObj_H
#define CQSandboxShlibObj_H

#include <CQSandboxObject3D.h>

class CShLib;

namespace CQSandbox {

class Shlib3DObj : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  Shlib3DObj(Canvas3D *canvas, const QString &libName);
 ~Shlib3DObj();

  const char *typeName() const override { return "shlib"; }

  void init() override;

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  bool exec(const QString &op, const QStringList &args, QVariant &res) override;

 private:
  QString libName_;
  QString libPath_;
  CShLib* shlib_ { nullptr };

  void* initProc_ { nullptr };
  void* getProc_  { nullptr };
  void* setProc_  { nullptr };
  void* execProc_ { nullptr };
};

}

#endif
