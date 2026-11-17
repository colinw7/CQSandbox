#ifndef CQSandboxShaderToy3DObj_H
#define CQSandboxShaderToy3DObj_H

#include <CQSandboxObject3D.h>

namespace CQSandbox {

class ShaderToyProgram;

class ShaderToy3DObj : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  ShaderToy3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "shader_toy"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void updateShaders();

  void tick() override;

  CBBox3D calcBBox() override { return bbox_; }

  void loadBuffer() override;

  void render() override;

 private:
  ShaderToyProgram* shaderToyProgram_ { nullptr };
};

}

#endif
