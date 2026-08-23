#ifndef CQSandboxGeom_H
#define CQSandboxGeom_H

#include <QPointF>
#include <QRectF>

#include <optional>

namespace CQSandbox {

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

}

#endif
