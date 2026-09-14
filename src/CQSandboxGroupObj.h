#ifndef CQSandboxGroupObj_H
#define CQSandboxGroupObj_H

#include <CQSandboxObject2D.h>

#include <CWindowRange2D.h>

namespace CQSandbox {

class GroupObj : public Object2D {
  Q_OBJECT

 public:
  using Objects = std::vector<Object2D *>;

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  GroupObj(Canvas2D *canvas, const Rect2D &rect);

  const char *typeName() const override { return "group"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  const CDisplayRange2D &displayRange() const { return displayRange_; }

  const Objects &objects() const { return objects_; }

  Rect2D calcRect() const override;

  void draw(QPainter *) override;

  void addObject(Object2D *obj);
  void removeObject(Object2D *obj);

  Rect2D rectToPixel(const Rect2D &p) const;
  Point2D pointToPixel(const Point2D &p) const;

//Point2D pointToWindow(const Point2D &p) const override;

 Q_SIGNALS:
  void objectsChanged();

 protected:
  Rect2D rect_;

  CDisplayRange2D displayRange_;
  Objects         objects_;
};

}

#endif
