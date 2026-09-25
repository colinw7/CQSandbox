#ifndef CQSandboxObject2D_H
#define CQSandboxObject2D_H

#include <CQSandboxGeom.h>
#include <CQSandboxAnim.h>

#include <QObject>
#include <QPainterPath>
#include <QPen>
#include <QVariant>

#include <tcl/tcl.h>

namespace CQSandbox {

class Canvas2D;
class Group2DObj;
class AnimateBrush;

class Object2D : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString id       READ id)
  Q_PROPERTY(bool    visible  READ isVisible  WRITE setVisible)
  Q_PROPERTY(bool    selected READ isSelected WRITE setSelected)
  Q_PROPERTY(int     layer    READ layer      WRITE setLayer)
  Q_PROPERTY(double  xPos     READ xPos       WRITE setXPos)
  Q_PROPERTY(double  yPos     READ yPos       WRITE setYPos)

 public:
  enum class Type {
    NONE,
    ARROW,
    ASTAR,
    AXIS,
    BUTTON,
    CIRCLE,
    CSV,
    EDIT,
    GROUP,
    IMAGE,
    LINE,
    MATRIX,
    OBJ_ARRAY,
    OBJ_MATRIX,
    PALETTE,
    PARTICLE,
    PATH,
    POINT_LIST,
    RECT,
    RENDERER,
    SHLIB,
    TEXT,
    VECTOR,

    CLASS,
    INSTANCE
  };

  using TclObjs = std::vector<Tcl_Obj *>;

 public:
  Object2D(Canvas2D *canvas, Type type);

  Object2D(const Object2D &) = delete;
  Object2D &operator=(const Object2D &) = delete;

  Canvas2D *canvas() const { return canvas_; }

  //---

  virtual const char *typeName() const = 0;

  QString typeNameStr() const { return QString(typeName()); }

  Type type() const { return type_; }

  //---

  size_t ind() const { return ind_; }
  void setInd(size_t ind) { ind_ = ind; }

  const QString &id() const { return id_; }
  void setId(const QString &s) { id_ = s; }

  QString calcId() const;

  //---

  bool isVisible() const { return visible_; }
  void setVisible(bool b) { visible_ = b; Q_EMIT stateChanged(); }

  bool isSelected() const { return selected_; }
  void setSelected(bool b) { selected_ = b; Q_EMIT stateChanged(); }

  bool isAnimating() const { return animating_; }
  void setAnimating(bool b) { animating_ = b; }

  int layer() const { return layer_; }
  void setLayer(int i) { layer_ = i; }

  //---

  Group2DObj *group() const { return group_; }
  void setGroup(Group2DObj *group) { group_ = group; }

  //---

  virtual bool isTclCmd() const { return false; }

  virtual bool init();

  virtual bool getValue(const QString &name,  const QStringList &args, QVariant &res);
  virtual bool setValue(const QString &name, const QString &value, const QStringList &args);

  virtual bool exec(const QString &op, const QStringList &args, QVariant &res);

  virtual bool getTclValue(const QString &name, const TclObjs &args, Tcl_Obj* &res);
  virtual bool setTclValue(const QString &name, Tcl_Obj *value, const TclObjs &args);

  virtual bool execTcl(const QString &op, const TclObjs &args, Tcl_Obj* &res);

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

  double xPos() const { return position().value().x; }
  void setXPos(double x) { setPosition(AnimatePoint2D(CPoint2D(x, yPos()))); }

  double yPos() const { return position().value().y; }
  void setYPos(double y) { setPosition(AnimatePoint2D(CPoint2D(xPos(), y))); }

  virtual const AnimatePoint2D &position() const;
  virtual void setPosition(const AnimatePoint2D &p);

  //---

  virtual Rect2D calcRect() const { return Rect2D(); }

  virtual QPainterPath calcPath() const { return QPainterPath(); }

  Rect2D getBBox() const { return calcRect(); }

  //---

  virtual void draw(QPainter *) { }

  //---

  virtual bool step();

  virtual void move(int dx, int dy);

  virtual void press(int x, int y);
  virtual void click(int x, int y);

  virtual QString getCommandName() const;

  virtual Point2D pointToWindow(const Point2D &p) const;

  Point2D pointToPixel(const Point2D &p) const;

  Rect2D rectToWindow(const Rect2D &r) const;

 Q_SIGNALS:
  void stateChanged();

 protected:
  using NameValues = std::map<QString, QVariant>;

  Canvas2D* canvas_ { nullptr };
  Type      type_   { Type::NONE };
  size_t    ind_    { 0 };

  QString id_;
  bool    visible_  { true };
  bool    selected_ { false };

  bool stroked_   { true };
  bool filled_    { true };
  bool animating_ { false };

  int layer_ { -1 };

  AnimatePoint2D position_;

  QPen         pen_;
  AnimateBrush brush_;

  NameValues nameValues_;

  Group2DObj *group_ { nullptr };

  QString meta_;
};

}

#endif
