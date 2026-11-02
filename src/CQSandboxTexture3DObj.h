#ifndef CQSandboxTexture3DObj_H
#define CQSandboxTexture3DObj_H

#include <CQSandboxObject3D.h>

namespace CQSandbox {

class Texture;

class Texture3DObj : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  Texture3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "texture"; }

  Texture *texture() const { return texture_; }
  void setTexture(Texture *p) { texture_ = p; }

  //---

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  //---

  void init() override;

  //---

  void render() override { }

  void loadBuffer() override { }

 private:
  Texture *texture_ { nullptr };
};

}

#endif
