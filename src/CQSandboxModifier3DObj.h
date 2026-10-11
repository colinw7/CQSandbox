#ifndef CQSandboxModifier3DObj_H
#define CQSandboxModifier3DObj_H

#include <CQSandboxObject3D.h>

#include <CQGLBuffer.h>

namespace CQSandbox {

class ShaderProgram;

class Modifier3DObj : public Object3D {
  Q_OBJECT

 public:
  enum class ModifierType {
    NONE,
    ARRAY,
    MIRROR
  };

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  Modifier3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "modifier"; }

  //---

  const Object3D *object() const { return object_; }
  void setObject(Object3D *obj);

  //---

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  //---

  void init() override;

  CBBox3D calcBBox() override { return bbox_; }

  void loadBuffer() override { }

  void render() override { }

  //---

  void drawData(ShaderProgram *program, CQGLBuffer *buffer, const CQGLBuffer::DrawData &data,
                const CMatrix3DH &modelMatrix);

 private:
  ModifierType type_ { ModifierType::NONE };

  Object3D* object_ { nullptr };

  struct ArrayData {
    int      count  { 2 };
    CPoint3D offset { 1, 0, 0 };
  };

  ArrayData arrayData_;
};

}

#endif
