#ifndef CQSandboxQuadTree2DObj_H
#define CQSandboxQuadTree2DObj_H

#include <CQSandboxGroup2DObj.h>

#include <CQuadTree.h>

namespace CQSandbox {

class QuadTree2DObj : public Group2DObj {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  QuadTree2DObj(Canvas2D *canvas);

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 private:
  using QuadTree = CQuadTree<Object2D, Rect2D>;

  QuadTree quadTree_;
};

}

#endif
