#ifndef CQSandboxInstance2DObj_H
#define CQSandboxInstance2DObj_H

#include <CQSandboxObject2D.h>

namespace CQSandbox {

class Class2DObj;

class Instance2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, int objc, const Tcl_Obj **objv);

  Instance2DObj(Canvas2D *canvas, Class2DObj *classObj, const std::vector<Tcl_Obj *> &args);

  const char *typeName() const override { return "instance"; }

  bool init() override;

  bool isTclCmd() const override { return true; }

  bool getTclValue(const QString &name, const TclObjs &objs, Tcl_Obj* &res) override;
  bool setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &objs) override;

 protected:
  using NameValueMap = std::map<QString, Tcl_Obj *>;

  Class2DObj*            classObj_ { nullptr };
  std::vector<Tcl_Obj *> args_;
  NameValueMap           nameValue_;
};

}

#endif
