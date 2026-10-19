#ifndef CQSandboxBBox3DObj_H
#define CQSandboxBBox3DObj_H

#include <CQSandboxObject3D.h>
#include <CGLVector3D.h>

namespace CQSandbox {

class ShaderProgram;

class BBox3DObj : public Object3D {
  Q_OBJECT

 public:
  using Points = std::vector<CGLVector3D>;

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  BBox3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "bbox"; }

  //---

  Object3D *parentObj() const { return parentObj_; }
  void setParentObj(Object3D *o) { parentObj_ = o; }

  const CBBox3D &parentBBox() const { return parentBBox_; }
  void setParentBBox(const CBBox3D &v) { parentBBox_ = v; }

  //---

  void init() override;

  void initShader();

  CBBox3D calcBBox() override { return bbox_; }

  void loadBuffer() override;

  void render() override;

 protected:
  static ShaderProgram* s_program;

  Points points_;

  Object3D* parentObj_ { nullptr };
  CBBox3D   parentBBox_;
};

}

#endif
