#ifndef CQSandboxAnim_H
#define CQSandboxAnim_H

#include <CMathUtil.h>

#include <QColor>
#include <QBrush>

namespace CQSandbox {

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

}

#endif
