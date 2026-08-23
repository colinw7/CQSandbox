#ifndef CQSandboxObject_H
#define CQSandboxObject_H

#include <CQSandboxGeom.h>
#include <CQSandboxAnim.h>

#include <QObject>
#include <QPainterPath>
#include <QPen>
#include <QVariant>

namespace CQSandbox {

class Canvas;
class GroupObj;
class AnimateBrush;

class Object : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString id       READ id)
  Q_PROPERTY(bool    visible  READ isVisible  WRITE setVisible)
  Q_PROPERTY(bool    selected READ isSelected WRITE setSelected)

 public:
  Object(Canvas *canvas, size_t ind=0);

  Object(const Object &) = delete;
  Object &operator=(const Object &) = delete;

  Canvas *canvas() const { return canvas_; }

  //---

  virtual const char *typeName() const = 0;

  //---

  size_t ind() const { return ind_; }
  void setInd(size_t ind) { ind_ = ind; }

  const QString &id() const { return id_; }
  void setId(const QString &s) { id_ = s; }

  QString calcId() const;

  //---

  bool isVisible() const { return visible_; }
  void setVisible(bool b) { visible_ = b; }

  bool isSelected() const { return selected_; }
  void setSelected(bool b) { selected_ = b; }

  bool isAnimating() const { return animating_; }
  void setAnimating(bool b) { animating_ = b; }

  //---

  GroupObj *group() const { return group_; }
  void setGroup(GroupObj *group) { group_ = group; }

  //---

  virtual bool getValue(const QString &name,  const QStringList &args, QVariant &value);
  virtual bool setValue(const QString &name, const QString &value, const QStringList &args);

  virtual bool exec(const QString &, const QStringList &, QVariant &) { return false; }

  //---

  bool isStroked() const { return stroked_; }
  void setStroked(bool b) { stroked_ = b; }

  const QPen &pen() const { return pen_; }

  bool isFilled() const { return filled_; }
  void setFilled(bool b) { filled_ = b; }

  const AnimateBrush &brush() const { return brush_; }
  void setBrush(const AnimateBrush &b) { brush_ = b; }
  void setTargetBrush(const QBrush &b) { brush_.setTarget(b); }

  //---

  QVariant nameValue(const QString &name) const {
    auto pn = nameValues_.find(name);

    if (pn != nameValues_.end())
      return (*pn).second;
    else
      return QVariant();
  }

  void setNameValue(const QString &name, const QVariant &value) {
    nameValues_[name] = value;
  }

  //---

  virtual Rect calcRect() const { return Rect(); }

  virtual QPainterPath calcPath() const { return QPainterPath(); }

  Rect getBBox() const { return calcRect(); }

  //---

  virtual void draw(QPainter *) { }

  virtual bool step();

  virtual void move(int, int) { }

  virtual void press(int x, int y);
  virtual void click(int x, int y);

  QString getCommandName() const;

  virtual Point pointToWindow(const Point &p) const;

  Point pointToPixel(const Point &p) const;

  Rect rectToWindow(const Rect &r) const;

 protected:
  using NameValues = std::map<QString, QVariant>;

  Canvas* canvas_  { nullptr };
  size_t  ind_     { 0 };

  QString id_;
  bool    visible_  { true };
  bool    selected_ { false };

  bool stroked_   { true };
  bool filled_    { true };
  bool animating_ { false };

  QPen         pen_;
  AnimateBrush brush_;

  NameValues nameValues_;

  GroupObj *group_ { nullptr };

  QString meta_;
};

}

#endif
