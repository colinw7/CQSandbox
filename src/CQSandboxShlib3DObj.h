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

  //---

  bool isTclCmd() const override { return true; }

  bool getTclValue(const QString &name, const TclObjs &objs, Tcl_Obj* &res) override;
  bool setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &objs) override;

  bool execTcl(const QString &op, const TclObjs &objs, Tcl_Obj* &res) override;

  //---

  void loadBuffer() override { }

  void render() override { }

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
