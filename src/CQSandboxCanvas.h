#ifndef CQSandbox_H
#define CQSandbox_H

#include <CTclUtil.h>
//#include <CDisplayRange2D.h>
#include <CWindowRange2D.h>
#include <CMathUtil.h>
#include <CRGBA.h>
#include <CQuadTree.h>
#include <CArray2D.h>

#include <CPSysSystem.h>
#include <CPSysParticle.h>

#include <QFrame>
#include <QVariant>
#include <QPen>
#include <QBrush>
#include <QPainterPath>

#include <optional>

class CQArrow;
class CQAxis;
class CQCsvModel;

class QTimer;

namespace CQSandbox {

class App;
class Canvas;
class Particle;
class Viewport;

enum class Units {
  WINDOW,
  PIXEL
};

struct Coord {
  Coord() { }

  Coord(double v, const Units &u=Units::WINDOW) :
   value(v), units(u) {
  }

  double value { 0.0 };
  Units  units { Units::WINDOW };
};

using OptCoord = std::optional<Coord>;

struct Point {
  static Point makePixel(double x, double y) {
    Point p;

    p.x.value = x; p.x.units = Units::PIXEL;
    p.y.value = y; p.y.units = Units::PIXEL;

    return p;
  }

  static Point makePixel(const QPointF &p) {
    return makePixel(p.x(), p.y());
  }

  static Point makeWindow(double x, double y) {
    Point p;

    p.x.value = x; p.x.units = Units::WINDOW;
    p.y.value = y; p.y.units = Units::WINDOW;

    return p;
  }

  static Point makeWindow(const QPointF &p) {
    return makeWindow(p.x(), p.y());
  }

  Point() { }

  Point(const Coord &c1, const Coord &c2) :
   x(c1), y(c2) {
  }

  QPointF qpoint() const {
    return QPointF(x.value, y.value);
  }

  Coord x;
  Coord y;
};

struct Rect {
  static Rect makeWindow(const QRectF &r) {
    return Rect(Point::makeWindow(r.left (), r.top   ()),
                Point::makeWindow(r.right(), r.bottom()));
  }

  Rect() { }

  Rect(double x1, double y1, double x2, double y2) :
   ll(x1, y1), ur(x2, y2) {
  }

  Rect(const Point &p1, const Point &p2) :
   ll(p1), ur(p2) {
  }

  QRectF qrect() const {
    auto x1 = std::min(ll.x.value, ur.x.value);
    auto y1 = std::min(ll.y.value, ur.y.value);
    auto x2 = std::max(ll.x.value, ur.x.value);
    auto y2 = std::max(ll.y.value, ur.y.value);

    return QRectF(x1, y1, x2 - x1, y2 - y1);
  }

  Point center() const {
    return Point((ll.x.value + ur.x.value)/2.0, (ll.y.value + ur.y.value)/2.0);
  }

  double getLeft  () const { return ll.x.value; }
  double getRight () const { return ur.x.value; }
  double getBottom() const { return ll.y.value; }
  double getTop   () const { return ur.y.value; }

  Point ll;
  Point ur;
};

//---

template<typename T>
class AnimateData {
 public:
  AnimateData() { }

  AnimateData(const T &init, const T &target) :
   value_(init), init_(init), target_(target) {
  }

  virtual ~AnimateData() { }

  const T &value() const { return value_; }
  void setValue(const T &t) { value_ = t; init_ = value_; }

  const T &target() const { return target_; }
  void setTarget(const T &t) { target_ = t; init_ = value_; step_ = 0; }

  size_t steps() const { return steps_; }
  void setSteps(size_t n) { steps_ = n; }

  virtual bool step() = 0;

  double delta() const {
    if (steps_ > 0)
      return CMathUtil::map(step_, 0, steps_ - 1, 0.0, 1.0);
    else
      return 0.0;
  }

 protected:
  T      value_;
  T      init_;
  T      target_;
  size_t step_  { 0 };
  size_t steps_ { 10 };
};

//---

class AnimateColor : public AnimateData<QColor> {
 public:
  AnimateColor() { }

  AnimateColor(const QColor &init, const QColor &target=QColor()) :
   AnimateData(init, target) {
    value_ = init;
    init_ = init;
  }

  bool step() override {
    if (step_ < steps_) {
      value_ = interpColor(init_, target_, delta());

      ++step_;
    }

    return (step_ < steps_);
  }

  static QColor interpColor(const QColor &c1, QColor &c2, double d) {
    auto r1 = c1.redF();
    auto g1 = c1.greenF();
    auto b1 = c1.blueF();

    auto r2 = c2.redF();
    auto g2 = c2.greenF();
    auto b2 = c2.blueF();

    auto r = CMathUtil::map(d, 0.0, 1.0, r1, r2);
    auto g = CMathUtil::map(d, 0.0, 1.0, g1, g2);
    auto b = CMathUtil::map(d, 0.0, 1.0, b1, b2);

    return QColor::fromRgbF(r, g, b);
  }
};

//---

class AnimateBrush : public AnimateData<QBrush> {
 public:
  AnimateBrush() { }

  AnimateBrush(const QBrush &init, const QBrush &target=QBrush()) :
   AnimateData(init, target) {
    value_ = init;
    init_ = init;
  }

  bool step() override {
    if (step_ < steps_) {
      value_ = interpBrush(init_, target_, delta());

      ++step_;
    }

    return (step_ < steps_);
  }

  static QBrush interpBrush(const QBrush &brush1, QBrush &brush2, double d) {
    auto c1 = brush1.color();
    auto c2 = brush2.color();

    auto r1 = c1.redF();
    auto g1 = c1.greenF();
    auto b1 = c1.blueF();
    auto a1 = c1.alphaF();

    auto r2 = c2.redF();
    auto g2 = c2.greenF();
    auto b2 = c2.blueF();
    auto a2 = c2.alphaF();

    auto r = CMathUtil::map(d, 0.0, 1.0, r1, r2);
    auto g = CMathUtil::map(d, 0.0, 1.0, g1, g2);
    auto b = CMathUtil::map(d, 0.0, 1.0, b1, b2);
    auto a = CMathUtil::map(d, 0.0, 1.0, a1, a2);

    auto c = QColor::fromRgbF(r, g, b);

    c.setAlphaF(a);

    return QBrush(c);
  }
};

//---

class AnimatePoint : public AnimateData<Point> {
 public:
  AnimatePoint(const Point &init, const Point &target=Point()) :
   AnimateData(init, target) {
    value_ = init;
    init_  = init;
  }

  bool step() override {
    if (step_ < steps_) {
      value_ = interpPoint(init_, target_, delta());

      ++step_;
    }

    return (step_ < steps_);
  }

  static Point interpPoint(const Point &p1, Point &p2, double d) {
    auto x1 = p1.x.value;
    auto y1 = p1.y.value;

    auto x2 = p2.x.value;
    auto y2 = p2.y.value;

    Point p;

    p.x.value = CMathUtil::map(d, 0.0, 1.0, x1, x2);
    p.y.value = CMathUtil::map(d, 0.0, 1.0, y1, y2);

    return p;
  }
};

//---

class AnimateCoord : public AnimateData<Coord> {
 public:
  AnimateCoord(const Coord &init, const Coord &target=Coord()) :
   AnimateData(init, target) {
    value_ = init;
    init_  = init;
  }

  bool step() override {
    if (step_ < steps_) {
      value_ = interpCoord(init_, target_, delta());

      ++step_;
    }

    return (step_ < steps_);
  }

  static Coord interpCoord(const Coord &c1, Coord &c2, double d) {
    auto r1 = c1.value;
    auto r2 = c2.value;

    Coord c;

    c.units = c1.units;
    c.value = CMathUtil::map(d, 0.0, 1.0, r1, r2);

    return c;
  }
};

//---

class GroupObj;

class Object : public QObject {
  Q_OBJECT

 public:
  Object(Canvas *canvas, size_t ind=0);

  Canvas *canvas() const { return canvas_; }

  virtual const char *typeName() const = 0;

  //---

  size_t ind() const { return ind_; }
  void setInd(size_t ind) { ind_ = ind; }

  const QString &id() const { return id_; }
  void setId(const QString &s) { id_ = s; }

  QString calcId() const;

  bool isVisible() const { return visible_; }
  void setVisible(bool b) { visible_ = b; }

  bool isAnimating() const { return animating_; }
  void setAnimating(bool b) { animating_ = b; }

  //---

  GroupObj *group() const { return group_; }
  void setGroup(GroupObj *group) { group_ = group; }

  //---

  virtual QVariant getValue(const QString &name,  const QStringList &args);
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
  bool    visible_ { true };

  bool stroked_ { true };
  bool filled_  { true };

  QPen         pen_;
  AnimateBrush brush_;

  bool animating_ { false };

  NameValues nameValues_;

  GroupObj *group_ { nullptr };

  QString meta_;
};

using Objects = std::vector<Object *>;

//---

class GroupObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  GroupObj(Canvas *canvas, const Rect &rect);

  const char *typeName() const override { return "group"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
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

//---

class RendererObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  RendererObj(Canvas *canvas);

  const char *typeName() const override { return "renderer"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  bool exec(const QString &op, const QStringList &args, QVariant &res) override;

 private:
  QBrush brush_;
  QPen   pen_;
  QFont  font_;
};

//---

class CirclesMgr;

class CirclesGroupObj : public GroupObj {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  CirclesGroupObj(Canvas *canvas, const Rect &rect);

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 protected:
  CirclesMgr *mgr_ { nullptr };
};

//---

class QuadTreeObj : public GroupObj {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  QuadTreeObj(Canvas *canvas);

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 private:
  using QuadTree = CQuadTree<Object, Rect>;

  QuadTree quadTree_;
};

//---

class RectObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  RectObj(Canvas *canvas, const Rect &rect);

  const char *typeName() const override { return "rect"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Rect rect_;
};

//---

class CircleObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  CircleObj(Canvas *canvas, const Point &center, const Coord &radius);

  const char *typeName() const override { return "circle"; }

  const AnimatePoint &center() const { return center_; }
  void setCenter(const AnimatePoint &c) { center_ = c; }
  void setTargetCenter(const Point &c) { center_.setTarget(c); }

  const AnimateCoord &radius() const { return radius_; }
  void setRadius(const AnimateCoord &r) { radius_ = r; }
  void setTargetRadius(const Coord &r) { radius_.setTarget(r); }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  bool step() override;

  void draw(QPainter *) override;

 protected:
  AnimatePoint center_;
  AnimateCoord radius_;
};

//---

class LineObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  LineObj(Canvas *canvas, const Point &p1, const Point &p2);

  const char *typeName() const override { return "line"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Point p1_;
  Point p2_;
};

//---

class TextObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  TextObj(Canvas *canvas, const Point &pos, const QString &text);

  const char *typeName() const override { return "text"; }

  const Point &position() const { return pos_; }
  void setPosition(const Point &v) { pos_ = v; }

  const QString &text() const { return text_; }
  void setText(const QString &s) { text_ = s; }

  const QFont &font() const { return font_; }
  void setFont(const QFont &f) { font_ = f; }

  bool isHtml() const { return html_; }
  void setHtml(bool b) { html_ = b; }

  const Qt::Alignment &align() const { return align_; }
  void setAlign(const Qt::Alignment &v) { align_ = v; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Point         pos_;
  QString       text_;
  QFont         font_;
  QPen          border_;
  Qt::Alignment align_ { Qt::AlignCenter };
  bool          html_  { false };
};

//---

class ImageObj : public Object {
  Q_OBJECT

 public:
  enum Position {
    TOP_LEFT,
    CENTER,
    RECT
  };

  static bool create(Canvas *canvas, const QStringList &args);

  ImageObj(Canvas *canvas, const Point &pos, const QImage &image);

  const char *typeName() const override { return "image"; }

  const QImage &image() const { return image_; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Point    pos_;
  Rect     rect_;
  Position posType_ { Position::TOP_LEFT };
  QImage   image_;
};

//---

class PathObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  PathObj(Canvas *canvas, const QPainterPath &path);

  const char *typeName() const override { return "path"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

 protected:
  QPainterPath path_;
};

//---

class PointListObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  PointListObj(Canvas *canvas, const Coord &radius);

  const char *typeName() const override { return "pointList"; }

  const AnimateCoord &radius() const { return radius_; }
  void setRadius(const AnimateCoord &r) { radius_ = r; }
  void setTargetRadius(const Coord &r) { radius_.setTarget(r); }

  bool isConnected() const { return connected_; }
  void setConnected(bool b) { connected_ = b; }

  bool isShowPoints() const { return showPoints_; }
  void setShowPoints(bool b) { showPoints_ = b; }

  bool isFillUnder() const { return fillUnder_; }
  void setFillUnder(bool b) { fillUnder_ = b; }

  const OptCoord &fillUnderY() const { return fillUnderY_; }
  void setFillUnderY(const OptCoord &v) { fillUnderY_ = v; }

  double angle() const { return angle_; }
  void setAngle(double r) { angle_ = r; }

  double scale() const { return scale_; }
  void setScale(double r) { scale_ = r; }

  const Point &center() const { return center_; }
  void setCenter(const Point &o) { center_ = o; }

  const Point &offset() const { return offset_; }
  void setOffset(const Point &o) { offset_ = o; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  QPainterPath calcPath() const override { return path_; }

  bool step() override;

  void draw(QPainter *) override;

 protected:
  using Points = std::vector<Point>;

  Points       points_;
  AnimateCoord radius_;
  bool         connected_  { false };
  bool         showPoints_ { false };
  bool         fillUnder_  { false };
  OptCoord     fillUnderY_;
  double       angle_      { 0.0 };
  double       scale_      { 1.0 };
  Point        center_     { 0, 0 };
  Point        offset_     { 0, 0 };
  QPainterPath path_;
};

//---

class ArrowObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  ArrowObj(Canvas *canvas, const Point &p1, const Point &p2);

  const char *typeName() const override { return "arrow"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Point p1_;
  Point p2_;

  CQArrow *arrow_ { nullptr };
};

//---

class AxisObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  AxisObj(Canvas *canvas, const Point &pos, const Coord &len);

  const char *typeName() const override { return "axis"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Point pos_;
  Coord len_;

  CQAxis *axis_ { nullptr };
};

//---

class ParticleObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  ParticleObj(Canvas *canvas, const Point &pos);

  const char *typeName() const override { return "particle"; }

  const Particle *particle() const { return particle_; }
  void setParticle(Particle *p);

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

 protected:
  Point     pos_;
  Particle* particle_ { nullptr };
};

//---

class VectorObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  VectorObj(Canvas *canvas);

  const char *typeName() const override { return "vector"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 protected:
  CVector2D v_;
};

//---

class ArrayObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  ArrayObj(Canvas *canvas, uint dim0, uint dim1);
  ArrayObj(Canvas *canvas, const CArray2D<double> &a);

  const char *typeName() const override { return "array"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 protected:
  CArray2D<double> a_;
};

//---

class CsvObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  CsvObj(Canvas *canvas, const QString &filename);

  const char *typeName() const override { return "csv"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  bool exec(const QString &op, const QStringList &args, QVariant &res) override;

 protected:
  QString filename_;

  CQCsvModel *csv_ { nullptr };
};

//---

class EditObj : public Object {
  Q_OBJECT

 public:
  EditObj(Canvas *canvas, const QString &name);

  const char *typeName() const override { return "edit"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

 protected:
  QString name_;
  QString proc_;
};

//---

class RealEdit : public EditObj {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  RealEdit(Canvas *canvas, const Point &p, const QString &name);

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

  void move(int, int) override;

 protected:
  Point  p_;
  double minValue_ { 0.0 };
  double maxValue_ { 1.0 };
};

//---

class IntegerEdit : public EditObj {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  IntegerEdit(Canvas *canvas, const Point &p, const QString &name);

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

  void move(int, int) override;

  void click(int x, int y) override;

 protected:
  int getIValue() const;
  void setIValue(int i);

 protected:
  Point p_;
  int   minValue_ { -9999 };
  int   maxValue_ { 9999 };
  QRect lrect_;
  QRect rrect_;
};

//---

class ButtonObj : public Object {
  Q_OBJECT

 public:
  static bool create(Canvas *canvas, const QStringList &args);

  ButtonObj(Canvas *canvas, const Point &p, const QString &name);

  const char *typeName() const override { return "button"; }

  QVariant getValue(const QString &name, const QStringList &args) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect calcRect() const override;

  void draw(QPainter *) override;

  void click(int x, int y) override;

 protected:
  Point   p_;
  QString name_;
  QString proc_;
};

//---

class ParticleSystem : public CPSysSystem {
 public:
  ParticleSystem();

  CPSysParticle *makeParticle(double mass=1.0, double x=0.0, double y=0.0, double z=0.0) override;

  const Object *particleObj() const { return particleObj_; }
  void setParticleObj(Object *p) { particleObj_ = p; }

 private:
  Object* particleObj_ { nullptr };
};

class Particle : public CPSysParticle {
 public:
  using OptColor = std::optional<CRGBA>;
  using OptPoint = std::optional<CPoint2D>;
  using OptSize  = std::optional<CSize2D>;

 public:
  Particle(double mass=1.0);

  const OptColor &color() const { return color_; }
  void setColor(const OptColor &v) { color_ = v; }

  const QImage &image() const { return image_; }
  void setImage(const QImage &i) { image_ = i; }

  double size() const { return size_; }
  void setSize(double r) { size_ = r; }

  double alpha() const { return alpha_; }
  void setAlpha(double r) { alpha_ = r; }

  double angle() const { return angle_; }
  void setAngle(double r) { angle_ = r; }

  const OptPoint &tpos() const { return tpos_; }
  void setTPos(const OptPoint &v) { tpos_ = v; }

  const OptSize &tsize() const { return tsize_; }
  void setTSize(const OptSize &v) { tsize_ = v; }

  const std::string &meta() const { return meta_; }
  void setMeta(const std::string &s) { meta_ = s; }

  const Object *obj() const { return obj_; }
  void setObj(Object *p) { obj_ = p; }

  void updateParticle() override;

 private:
  OptColor    color_;
  QImage      image_;
  double      size_    { 1.0 };
  double      alpha_   { 1.0 };
  double      angle_   { 0.0 };
  OptPoint    tpos_;
  OptSize     tsize_;
  std::string meta_;

  Object *obj_ { nullptr };
};

//---

class Canvas : public QFrame {
  Q_OBJECT

 public:
  using Viewports = std::vector<Viewport *>;

 public:
  Canvas(App *app);

  App* app() const { return app_; }

  void resizeEvent(QResizeEvent *) override;

  void paintEvent(QPaintEvent *) override;

  void drawStep();

  void drawParticle(QPainter *, Particle *);

  void fadeImage(QImage &image1, QImage &image2, double f);

  QPainter *painter() const { return painter_; }

  //---

  void mousePressEvent  (QMouseEvent *) override;
  void mouseMoveEvent   (QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;

  void keyPressEvent  (QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;

  bool getKeyPressed(const QString &key) const;

  QString getKeyString(QKeyEvent *e) const;

  //---

  Object *getObjectAtPos(const QPoint &pos) const;
  Object *getObjectByName(const QString &name) const;

  void init();

  void addCommands();

  void play();
  void pause();
  void step();
  void stepInit(bool &buffered);

  void drawBuffered();

  QString addNewObject(Object *obj);

  void addObject(Object *obj);
  void removeObject(Object *obj);

  void createObjCommand(Object *obj);

  Point pointToWindow(const Point &p) const;

  Rect rectToPixel(const Rect &rect) const;
  Point pointToPixel(const Point &p) const;

  QSizeF pixelSizeToWindow(const QSizeF &psize) const;

  Viewport *currentViewport() const;

  Viewport *addViewport();

  QVariant getPaletteValue(const QString &);
  void setPaletteValue(const QString &, const QString &);

  QVariant getStyleValue(const QString &);
  void setStyleValue(const QString &, const QString &);

  const QPen   &stylePen  () const { return stylePen_  ; }
  const QBrush &styleBrush() const { return styleBrush_; }

  //---

  ParticleSystem *psys() const { return psys_; }

 protected:
  static int objectCommandProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int viewportCommandProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int canvasProc (void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);
  static int paletteProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);
  static int styleProc  (void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int viewportProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int drawPointProc(void *clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv);

  static int fmulProc (void *, Tcl_Interp *interp, int objc, const Tcl_Obj **objv);
  static int fmaProc  (void *, Tcl_Interp *interp, int objc, const Tcl_Obj **objv);
  static int hypotProc(void *, Tcl_Interp *interp, int objc, const Tcl_Obj **objv);

  static int uiProc(void *, Tcl_Interp *interp, int objc, const Tcl_Obj **objv);

  QVariant getValue(const QString &, const QStringList &);
  bool setValue(const QString &, const QString &, const QStringList &);
  bool exec(const QString &, const QStringList &, QVariant &);

  void updatePixelRanges();

 Q_SIGNALS:
  void objectsChanged();

 protected Q_SLOTS:
  void timerSlot();
  void stepTimerSlot();
  void drawTimerSlot();

 protected:
  App* app_ { nullptr };

  ParticleSystem *psys_ { nullptr };

  QTimer *timer_      { nullptr };
  QTimer *stepTimer_  { nullptr };
  QTimer *drawTimer_  { nullptr };
  bool    running_    { false };
  uint    timerTicks_ { 30 };
  uint    ticks_      { 0 };

  size_t lastInd_ { 0 };

  bool initialized_ { false };
  bool inited_      { false };
  bool initRun_     { false };

  QPen   stylePen_;
  QBrush styleBrush_;

  QPoint  pressPos_;
  Object* pressObj_ { nullptr };
  QPoint  motionPos_;
  bool    pressed_  { false };

  QPainter *painter_            { nullptr };
  bool      drawing_            { false };
  bool      drawBufferedNeeded_ { false };

  bool   buffered_    { false };
  bool   blend_       { false };
  double blendFactor_ { 0.95 };
  QImage bufferImage1_;
  QImage bufferImage2_;
  int    pixelWidth_  { 1 };
  int    pixelHeight_ { 1 };

  //--

  Viewports viewports_;
  Viewport* currentViewport_ { nullptr };
  QString   currentViewportName_;

  Objects allObjects_;

  //---

  using KeyPressed = std::map<QString, bool>;

  KeyPressed keyPressed_;
};

}

#endif
