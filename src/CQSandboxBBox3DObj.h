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

  void init() override;

  void initShader();

  CBBox3D calcBBox() override { return bbox_; }

  void updateGL();

  void render() override;

  const CBBox3D &parentBBox() const { return parentBBox_; }
  void setParentBBox(const CBBox3D &v) { parentBBox_ = v; }

 protected:
  static ShaderProgram* s_program;

  Points points_;

  CBBox3D parentBBox_;
};

}

#endif
