#ifndef CQSandboxShlibObj_H
#define CQSandboxShlibObj_H

#include <CQSandboxObject2D.h>

class CShLib;

namespace CQSandbox {

class Shlib2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  Shlib2DObj(Canvas2D *canvas, const QString &libName);
 ~Shlib2DObj();

  const char *typeName() const override { return "shlib"; }

  bool init() override;

  bool isTclCmd() const override { return true; }

  bool getTclValue(const QString &name, const TclObjs &objs, Tcl_Obj* &res) override;
  bool setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &objs) override;

  bool execTcl(const QString &op, const TclObjs &objs, Tcl_Obj* &res) override;

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
