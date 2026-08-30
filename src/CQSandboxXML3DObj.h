#ifndef CQSandboxXML3DObj_H
#define CQSandboxXML3DObj_H

#include <CQSandboxObject3D.h>

class CXML;
class CXMLTag;

namespace CQSandbox {

class Xml3DObj : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  Xml3DObj(Canvas3D *canvas, const QString &filename);
 ~Xml3DObj();

  const char *typeName() const override { return "csv"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  bool exec(const QString &op, const QStringList &args, QVariant &res) override;

  //---

  void init() override;

 protected:
  QString filename_;

  CXML*    xml_ { nullptr };
  CXMLTag* tag_ { nullptr };
};

}

#endif
