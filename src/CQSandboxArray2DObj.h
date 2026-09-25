#ifndef CQSandboxArray2DObj_H
#define CQSandboxArray2DObj_H

#include <CQSandboxObject2D.h>

namespace CQSandbox {

class ObjArray2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  ObjArray2DObj(Canvas2D *canvas, uint dim);

  const char *typeName() const override { return "obj_array"; }

  bool isTclCmd() const override { return true; }

  bool getTclValue(const QString &name, const TclObjs &objs, Tcl_Obj* &res) override;
  bool setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &objs) override;

 protected:
  using Values = std::vector<Tcl_Obj *>;

  Values values_;
};

class ObjMatrix2DObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  ObjMatrix2DObj(Canvas2D *canvas, uint dim1, uint dim2);

  const char *typeName() const override { return "obj_matrix"; }

  bool isTclCmd() const override { return true; }

  bool getTclValue(const QString &name, const TclObjs &objs, Tcl_Obj* &res) override;
  bool setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &objs) override;

 protected:
  using Values      = std::vector<Tcl_Obj *>;
  using ValuesArray = std::vector<Values>;

  ValuesArray values_;
};

}

#endif
