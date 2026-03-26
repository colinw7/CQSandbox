#ifndef CQSandboxDungeon3DObj_H
#define CQSandboxDungeon3DObj_H

#ifdef CQSANDBOX_DUNGEON
#include <CQSandboxObject3D.h>

class CQGLTexture;

class CDungeon;

namespace CQSandbox {

class Dungeon3DObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Dungeon3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "Dungeon"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void tick() override;

  void updateObjs();

  void render() override;

  void updatePlayerCamera(bool isGame);

  void setTexture(const QString &id, const QString &filename);
  CQGLTexture *getTexture(const QString &id) const;

 private:
  using Textures = std::map<QString, CQGLTexture *>;

  CDungeon* dungeon_ { nullptr };

  double s_  { 1.0 };
  double dx_ { 1.0 };
  double dy_ { 1.0 };
  double dz_ { 1.0 };

  Group3DObj *group_ { nullptr };

  Textures textures_;
};

}
#endif

#endif
