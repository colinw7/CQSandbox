#ifndef CQSandboxShader3DObj_H
#define CQSandboxShader3DObj_H

#include <CQSandboxObject3D.h>

namespace CQSandbox {

class ShaderProgram;

class Shader3DObj : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  Shader3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "shader"; }

  void init() override;

  //--

  ShaderProgram *shaderProgram() const { return shaderProgram_; }

  //--

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  bool exec(const QString &op, const QStringList &args, QVariant &res) override;

  //--

  void updateShaders();

  void loadBuffer() override { }

  void render() override { }

 private:
  ShaderProgram* shaderProgram_ { nullptr };
};

}

#endif
