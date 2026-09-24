#ifndef CQSandboxClass2DObj_H
#define CQSandboxClass2DObj_H

#include <CQSandboxObject2D.h>

namespace CQSandbox {

class Instance2DObj;

class Class2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  Class2DObj(Canvas2D *canvas, const QString &name);

  const char *typeName() const override { return "class"; }

  const QString &name() const { return name_; }

  bool addMethod(const QString &name, const QString &args, const QString &body);

  bool invokeMethod(const QString &name, const QString &instanceName,
                    const std::vector<Tcl_Obj *> &args, Tcl_Obj* &res);

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 protected:
  struct MethodData {
    QStringList args;
    QString     body;
    QString     procName;
    QString     callCmd;
    bool        bodySet { false };

    MethodData() { }

    MethodData(const QStringList &a, const QString &b) :
     args(a), body(b) {
    }
  };

  using Methods = std::map<QString, MethodData>;

  QString name_;

  Methods methods_;
};

}

#endif
