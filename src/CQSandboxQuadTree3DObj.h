#ifndef CQSandboxQuadTree3DObj_H
#define CQSandboxQuadTree3DObj_H

#include <CQSandboxGroup3DObj.h>

#include <CQuadTree.h>

namespace CQSandbox {

class QuadTree3DObj : public Group3DObj {
  Q_OBJECT

 public:
  static Object3D *create(Canvas3D *canvas, const QStringList &args);

  QuadTree3DObj(Canvas3D *canvas);

  const char *typeName() const override { return "quad_tree"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  //---

  void init() override;

 private:
  using QuadTree = CQuadTree<Object3D, Rect>;

  QuadTree quadTree_;
};

}

#endif
