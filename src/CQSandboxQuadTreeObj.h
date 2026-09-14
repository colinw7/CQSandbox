#ifndef CQSandboxQuadTreeObj_H
#define CQSandboxQuadTreeObj_H

#include <CQSandboxGroupObj.h>

#include <CQuadTree.h>

namespace CQSandbox {

class QuadTreeObj : public GroupObj {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  QuadTreeObj(Canvas2D *canvas);

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 private:
  using QuadTree = CQuadTree<Object2D, Rect2D>;

  QuadTree quadTree_;
};

}

#endif
