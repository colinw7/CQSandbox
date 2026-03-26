#ifndef CQSandboxGroup3DObj_H
#define CQSandboxGroup3DObj_H

#include <CQSandboxObject3D.h>

namespace CQSandbox {

class Group3DObj : public Object3D {
  Q_OBJECT

 public:
  using Objects = std::vector<Object3D *>;

 public:
  static bool create(Canvas3D *canvas, const QStringList &args);

  Group3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "Group"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  void init() override;

  void addObject(Object3D *obj);
  void removeObject(Object3D *obj);

  void render() override;

  CPoint3D origin() const override;

  void initOrigin();

  CBBox3D calcBBox() override;

  void setAngles(double xa, double ya, double za) override;

  void setModelMatrix(uint flags=ModelMatrixFlags::ALL) override;

 Q_SIGNALS:
  void objectsChanged();

 protected:
  Objects objects_;
};

}

#endif
