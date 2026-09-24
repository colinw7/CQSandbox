#ifndef CQSandboxArray2DObj_H
#define CQSandboxArray2DObj_H

#include <CQSandboxObject2D.h>

namespace CQSandbox {

class Array2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  Array2DObj(Canvas2D *canvas, uint dim);

  const char *typeName() const override { return "array"; }

  bool isTclCmd() const override { return true; }

  bool getTclValue(const QString &name, const TclObjs &objs, Tcl_Obj* &res) override;
  bool setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &objs) override;

 protected:
  std::vector<Tcl_Obj *> values_;
};

}

#endif
