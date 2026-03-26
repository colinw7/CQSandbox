#ifndef CQSandboxFieldRunners3DObj_H
#define CQSandboxFieldRunners3DObj_H

#ifdef CQSANDBOX_FIELD_RUNNERS
#include <CQSandboxObject3D.h>

class CQGLTexture;
class CFieldRunners;

namespace CQSandbox {

class FieldRunners3DObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  FieldRunners3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "FieldRunners"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void tick() override;

  void render() override;

 private:
  using Sprites  = std::vector<Sprite3DObj *>;
  using Textures = std::map<QString, CQGLTexture *>;

  CFieldRunners* runners_ { nullptr };

  Sprites  bgSprites_;
  Sprites  runnerSprites_;
  Textures textures_;
};

}
#endif

#endif
