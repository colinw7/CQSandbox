#ifndef CQSandboxAxis3DObj_H
#define CQSandboxAxis3DObj_H

#include <CQSandboxObject3D.h>

namespace CQSandbox {

class Path3DObj;
class Text3DObj;

class Axis3DObj : public Object3D {
  Q_OBJECT

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Axis3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "Axis"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void tick() override;

  void render() override;

 private:
  void updateObjects();

  void updateModelMatrix() override;

 private:
  using TextObjs = std::vector<Text3DObj *>;

  CGLVector3D start_ { 0, 0, 0 };
  CGLVector3D end_   { 1, 0, 0 };
  double      min_   { 0 };
  double      max_   { 1 };

  Path3DObj *path_ { nullptr };
  TextObjs   textObjs_;

  std::vector<double> ticks_;
};

}

#endif
