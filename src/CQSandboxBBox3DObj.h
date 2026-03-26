#ifndef CQSandboxBBox3DObj_H
#define CQSandboxBBox3DObj_H

#include <CQSandboxObject3D.h>

namespace CQSandbox {

class ShaderProgram;

class BBox3DObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  BBox3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "BBox"; }

  void init() override;

  void initShader();

  void updateGL();

  void render() override;

 protected:
  using Points = std::vector<CGLVector3D>;

  static ShaderProgram* s_program;

  Points points_;

  unsigned int pointsBufferId_ { 0 };
  unsigned int vertexArrayId_  { 0 };
};

}

#endif
