#ifndef CQSandboxGeom_H
#define CQSandboxGeom_H

#include <CPoint2D.h>

#include <QPointF>
#include <QRectF>

#include <optional>

namespace CQSandbox {

enum class Units {
  WINDOW,
  PIXEL
};

//---

struct Coord {
  Coord() { }

  Coord(double v, const Units &u=Units::WINDOW) :
   value(v), units(u) {
  }

  double value { 0.0 };
  Units  units { Units::WINDOW };
};

using OptCoord = std::optional<Coord>;

//---

struct Point2D {
  static Point2D makePixel(double x, double y) {
    Point2D p;

    p.x.value = x;
    p.y.value = y;

    p.setUnits(Units::PIXEL);

    return p;
  }

  static Point2D makePixel(const QPointF &p) {
    return makePixel(p.x(), p.y());
  }

  static Point2D makePixel(const CPoint2D &p) {
    return makePixel(p.x, p.y);
  }

  static Point2D makeWindow(double x, double y) {
    Point2D p;

    p.x.value = x;
    p.y.value = y;

    p.setUnits(Units::WINDOW);

    return p;
  }

  static Point2D makeWindow(const QPointF &p) {
    return makeWindow(p.x(), p.y());
  }

  static Point2D makeWindow(const CPoint2D &p) {
    return makeWindow(p.x, p.y);
  }

  Point2D() { }

  Point2D(const Coord &c1, const Coord &c2) :
   x(c1), y(c2) {
     assert(c1.units == c2.units);
  }

  QPointF qpoint() const {
    return QPointF(x.value, y.value);
  }

  CPoint2D point() const {
    return CPoint2D(x.value, y.value);
  }

  friend Point2D operator+(const Point2D &lhs, const Point2D &rhs) {
    assert(lhs.x.units == rhs.x.units);

    Point2D p;

    p.x.value = lhs.x.value + rhs.x.value;
    p.y.value = lhs.y.value + rhs.y.value;

    p.setUnits(lhs.x.units);

    return p;
  }

  Point2D &operator+=(const Point2D &rhs) {
    assert(x.units == rhs.x.units);

    x.value += rhs.x.value;
    y.value += rhs.y.value;

    return *this;
  }

  const Units &units() const { return x.units; }

  void setUnits(const Units &units) {
    x.units = units;
    y.units = units;
  }

  Coord x;
  Coord y;
};

//---

struct Rect2D {
  static Rect2D makeWindow(const QRectF &r) {
    return Rect2D(Point2D::makeWindow(r.left (), r.top   ()),
                  Point2D::makeWindow(r.right(), r.bottom()));
  }

  Rect2D() { }

  Rect2D(double x1, double y1, double x2, double y2) :
   ll(x1, y1), ur(x2, y2) {
  }

  Rect2D(const Point2D &p1, const Point2D &p2) :
   ll(p1), ur(p2) {
  }

  QRectF qrect() const {
    auto x1 = std::min(ll.x.value, ur.x.value);
    auto y1 = std::min(ll.y.value, ur.y.value);
    auto x2 = std::max(ll.x.value, ur.x.value);
    auto y2 = std::max(ll.y.value, ur.y.value);

    return QRectF(x1, y1, x2 - x1, y2 - y1);
  }

  Point2D center() const {
    Point2D c;

    c.x = (ll.x.value + ur.x.value)/2.0;
    c.y = (ll.y.value + ur.y.value)/2.0;

    c.setUnits(ll.units());

    return c;
  }

  double getLeft  () const { return ll.x.value; }
  double getRight () const { return ur.x.value; }
  double getBottom() const { return ll.y.value; }
  double getTop   () const { return ur.y.value; }

  void moveBy(const Point2D &d) {
    ll += d;
    ur += d;
  }

  const Units &units() const { return ll.units(); }

  Point2D ll;
  Point2D ur;
};

//---

}

#endif
