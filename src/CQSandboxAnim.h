#ifndef CQSandboxAnim_H
#define CQSandboxAnim_H

#include <CMathUtil.h>
#include <CPoint3D.h>

#include <QColor>
#include <QBrush>

namespace CQSandbox {

//---

template<typename T>
class AnimateData {
 public:
  enum class Style {
    ONE_SHOT,
    BOUNCE_ONCE,
    BOUNCE_ALWAYS
  };

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

  size_t step() const { return step_; }

  size_t steps() const { return steps_; }
  void setSteps(size_t n) { steps_ = n; }

  bool atStart() const { return (step_ == 0); }
  bool atEnd  () const { return (step_ == steps_); }

  bool canStep() const { return (step_ < steps_); }

  const Style &style() const { return style_; }
  void setStyle(const Style &v) { style_ = v; }

  void reset() { value_ = T(); init_ = T(); target_ = T(); step_ = 0; steps_ = 0; }

  //---

  virtual bool step() {
    if (canStep()) {
      updateValue();

      ++step_;

      if (atEnd() && style_ == Style::BOUNCE_ALWAYS) {
        std::swap(init_, target_);

        step_ = 0;
      }

      return canStep();
    }
    else
      return false;
  }

  virtual void updateValue() = 0;

  double delta() const {
    if (steps_ > 0)
      return CMathUtil::map(step_, 0, steps_ - 1, 0.0, 1.0);
    else
      return 0.0;
  }

 protected:
  T      value_  { };                 // current value
  T      init_   { };                 // init value
  T      target_ { };                 // target value
  size_t step_   { 0 };               // current step number
  size_t steps_  { 0 };               // max steps
  Style  style_  { Style::ONE_SHOT }; // behavior when at end
};

//---

class AnimateReal : public AnimateData<double> {
 public:
  AnimateReal() { }

  AnimateReal(double init, double target=0.0) :
   AnimateData(init, target) {
    value_ = init;
    init_  = init;
  }

  void updateValue() override {
    value_ = interpReal(init_, target_, delta());
  }

  static double interpReal(double r1, double r2, double d) {
    return CMathUtil::map(d, 0.0, 1.0, r1, r2);
  }
};

//---

class AnimateColor : public AnimateData<QColor> {
 public:
  AnimateColor() { }

  AnimateColor(const QColor &init, const QColor &target=QColor()) :
   AnimateData(init, target) {
    value_ = init;
    init_  = init;
  }

  void updateValue() override {
    value_ = interpColor(init_, target_, delta());
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
    init_  = init;
  }

  void updateValue() override {
    value_ = interpBrush(init_, target_, delta());
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

class AnimatePoint2D : public AnimateData<Point2D> {
 public:
  AnimatePoint2D(const Point2D &init, const Point2D &target=Point2D()) :
   AnimateData(init, target) {
    value_ = init;
    init_  = init;
  }

  void updateValue() override {
    value_ = interpPoint(init_, target_, delta());
  }

  static Point2D interpPoint(const Point2D &p1, Point2D &p2, double d) {
    auto x1 = p1.x.value;
    auto y1 = p1.y.value;

    auto x2 = p2.x.value;
    auto y2 = p2.y.value;

    Point2D p;

    p.x.value = CMathUtil::map(d, 0.0, 1.0, x1, x2);
    p.y.value = CMathUtil::map(d, 0.0, 1.0, y1, y2);

    return p;
  }
};

//---

class AnimatePoint3D : public AnimateData<CPoint3D> {
 public:
  AnimatePoint3D(const CPoint3D &init, const CPoint3D &target=CPoint3D()) :
   AnimateData(init, target) {
    value_ = init;
    init_  = init;
  }

  void updateValue() override {
    value_ = interpPoint(init_, target_, delta());
  }

  static CPoint3D interpPoint(const CPoint3D &p1, CPoint3D &p2, double d) {
    CPoint3D p;

    p.x = CMathUtil::map(d, 0.0, 1.0, p1.x, p2.x);
    p.y = CMathUtil::map(d, 0.0, 1.0, p1.y, p2.y);
    p.y = CMathUtil::map(d, 0.0, 1.0, p1.z, p2.z);

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

  void updateValue() override {
    value_ = interpCoord(init_, target_, delta());
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

}

#endif
