#ifndef CQSandboxQuadTreeObj_H
#define CQSandboxQuadTreeObj_H

#include <CQSandboxGroupObj.h>

#include <CQuadTree.h>

namespace CQSandbox {

class QuadTreeObj : public GroupObj {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  QuadTreeObj(Canvas *canvas);

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 private:
  using QuadTree = CQuadTree<Object, Rect>;

  QuadTree quadTree_;
};

}

#endif
