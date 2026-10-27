#ifndef CQSandboxGizmo3DObj_H
#define CQSandboxGizmo3DObj_H

#include <CQSandboxObject3D.h>

namespace CQSandbox {

class ShaderProgram;
class Shape3DObj;

class Gizmo3DObj : public Object3D {
  Q_OBJECT

 public:
  enum class EditType {
    NONE,
    SELECT,
    MOVE,
    SCALE,
    ROTATE
  };

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  static ShaderProgram* shaderProgram() { return s_program; }

  //---

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

  bool intersectRay(const CVector3D &p1, const CVector3D &p2,
                    CPoint3D &pi1, CPoint3D &pi2) const override;

  bool pressPoint(const CPoint2D &p) const;

  void loadBuffer() override;

  CBBox3D calcBBox() override;

  void render() override;

 private Q_SLOTS:
  void updateEditType();

 private:
  void updateObjects();

  void updateModelMatrix() override;

 private:
  struct ShapeData {
    Shape3DObj* shape  { nullptr };
    CPoint3D    angles;
    CPoint3D    offset;
    QColor      color;
  };

  static ShaderProgram* s_program;

  EditType editType_ { EditType::NONE };

  Object3D* parentObj_ { nullptr };

  ShapeData xCylinder_;
  ShapeData yCylinder_;
  ShapeData zCylinder_;

  ShapeData xCone_;
  ShapeData yCone_;
  ShapeData zCone_;

  ShapeData xCube_;
  ShapeData yCube_;
  ShapeData zCube_;

  ShapeData xTorus_;
  ShapeData yTorus_;
  ShapeData zTorus_;

  std::vector<ShapeData> shapes_;
};

}

#endif
