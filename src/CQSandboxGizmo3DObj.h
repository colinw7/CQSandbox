#ifndef CQSandboxGizmo3DObj_H
#define CQSandboxGizmo3DObj_H

#include <CQSandboxObject3D.h>

namespace CQSandbox {

class Shape3DObj;

class Gizmo3DObj : public Object3D {
  Q_OBJECT

 public:
  enum class EditType {
    NONE,
    MOVE,
    SCALE,
    ROTATE
  };

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  Gizmo3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "gizmo"; }

  //---

  const EditType &editType() const { return editType_; }
  void setEditType(const EditType &t);

  Object3D *parentObj() const { return parentObj_; }
  void setParentObj(Object3D *o);

  //---

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void loadBuffer() override;

  CBBox3D calcBBox() override;

  void render() override;

 private Q_SLOTS:
  void updateEditType();

 private:
  void updateObjects();

  void updateModelMatrix() override;

 private:
  EditType editType_ { EditType::NONE };

  Object3D* parentObj_ { nullptr };

  Shape3DObj* xCylinder_ { nullptr };
  Shape3DObj* yCylinder_ { nullptr };
  Shape3DObj* zCylinder_ { nullptr };

  Shape3DObj* xCone_ { nullptr };
  Shape3DObj* yCone_ { nullptr };
  Shape3DObj* zCone_ { nullptr };

  Shape3DObj* xTorus_ { nullptr };
  Shape3DObj* yTorus_ { nullptr };
  Shape3DObj* zTorus_ { nullptr };
};

}

#endif
