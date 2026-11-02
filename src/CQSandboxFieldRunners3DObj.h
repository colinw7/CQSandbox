#ifndef CQSandboxFieldRunners3DObj_H
#define CQSandboxFieldRunners3DObj_H

#ifdef CQSANDBOX_FIELD_RUNNERS
#include <CQSandboxObject3D.h>

class CFieldRunners;

namespace CQSandbox {

class Sprite3DObj;
class Texture;

class FieldRunners3DObj : public Object3D {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  FieldRunners3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "field_runners"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void loadBuffer() override { }

  void tick() override;

  void render() override;

 private:
  using Sprites  = std::vector<Sprite3DObj *>;
  using Textures = std::map<QString, Texture *>;

  CFieldRunners* runners_ { nullptr };

  Sprites  bgSprites_;
  Sprites  runnerSprites_;
  Textures textures_;
};

}
#endif

#endif
