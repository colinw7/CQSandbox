#ifndef CQSandboxJson3DObj_H
#define CQSandboxJson3DObj_H

#include <CQSandboxObject3D.h>

#include <CJson.h>

class CJson;

namespace CQSandbox {

class Json3DObj : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  Json3DObj(Canvas3D *canvas, const QString &filename);
 ~Json3DObj();

  const char *typeName() const override { return "csv"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  bool exec(const QString &op, const QStringList &args, QVariant &res) override;

  //---

  void init() override;

  CBBox3D calcBBox() override { return bbox_; }

 protected:
  QString filename_;

  CJson*        json_ { nullptr };
  CJson::ValueP value_;
};

}

#endif
