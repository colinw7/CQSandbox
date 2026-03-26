#ifndef CQSandboxSprite3DObj_H
#define CQSandboxSprite3DObj_H

#include <CQSandboxObject3D.h>

class CQGLBuffer;
class CQGLTexture;

namespace CQSandbox {

class ShaderProgram;

class Sprite3DObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Sprite3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "Sprite"; }

  void setTexture(CQGLTexture *texture);

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void updateBuffer();

  void tick() override;

  void render() override;

 private:
  void updateObjects();

  void updateModelMatrix() override;

 private:
  static ShaderProgram* s_program;

  CQGLBuffer*                buffer_ { nullptr };
  std::vector<CQGLTexture *> textures_;
  int                        textureNum_ { 0 };
  int                        textureStart_ { 0 };
  int                        textureEnd_ { -1 };
  double                     xv_ { 0.0 };
  double                     yv_ { 0.0 };
};

}

#endif
