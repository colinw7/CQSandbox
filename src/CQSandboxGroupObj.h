#ifndef CQSandboxGroupObj_H
#define CQSandboxGroupObj_H

#include <CQSandboxObject.h>

#include <CWindowRange2D.h>

namespace CQSandbox {

class GroupObj : public Object {
  Q_OBJECT

 public:
  using Objects = std::vector<Object *>;

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  GroupObj(Canvas *canvas, const Rect &rect);

  const char *typeName() const override { return "group"; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  const CDisplayRange2D &displayRange() const { return displayRange_; }

  const Objects &objects() const { return objects_; }

  Rect calcRect() const override;

  void draw(QPainter *) override;

  void addObject(Object *obj);
  void removeObject(Object *obj);

  Rect rectToPixel(const Rect &p) const;
  Point pointToPixel(const Point &p) const;

//Point pointToWindow(const Point &p) const override;

 Q_SIGNALS:
  void objectsChanged();

 protected:
  Rect rect_;

  CDisplayRange2D displayRange_;
  Objects         objects_;
};

}

#endif
