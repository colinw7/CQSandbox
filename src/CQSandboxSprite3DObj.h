#ifndef CQSandboxSprite3DObj_H
#define CQSandboxSprite3DObj_H

#include <CQSandboxObject3D.h>


namespace CQSandbox {

class Texture;
class ShaderProgram;

class Sprite3DObj : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  Sprite3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "sprite"; }

  Texture *texture() const;
  void setTexture(Texture *texture);

  Texture *currentTexture() const;

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void tick() override;

  CBBox3D calcBBox() override { return bbox_; }

  void loadBuffer() override;

  void render() override;

 private:
  void initShader();

  void updateObjects();

  void updateModelMatrix() override;

 private:
  static ShaderProgram* s_program;

  std::vector<Texture *> textures_;
  int                    textureNum_ { 0 };
  int                    textureStart_ { 0 };
  int                    textureEnd_ { -1 };
  double                 xv_ { 0.0 };
  double                 yv_ { 0.0 };
};

}

#endif
