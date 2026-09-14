#ifndef CQSandboxPointListObj_H
#define CQSandboxPointListObj_H

#include <CQSandboxObject2D.h>

#include <QPainterPath>

namespace CQSandbox {

class PointListObj : public Object2D {
  Q_OBJECT

 public:
  static bool create(Canvas2D *canvas, const QStringList &args);

  PointListObj(Canvas2D *canvas, const Coord &radius);

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

  const Point2D &center() const { return center_; }
  void setCenter(const Point2D &o) { center_ = o; }

  const Point2D &offset() const { return offset_; }
  void setOffset(const Point2D &o) { offset_ = o; }

  bool getValue(const QString &name, const QStringList &args, QVariant &value) override;
  bool setValue(const QString &name, const QString &value, const QStringList &args) override;

  Rect2D calcRect() const override;

  QPainterPath calcPath() const override { return path_; }

  bool step() override;

  void draw(QPainter *) override;

 protected:
  using Points = std::vector<Point2D>;

  Points       points_;
  AnimateCoord radius_;
  bool         connected_  { false };
  bool         showPoints_ { false };
  bool         fillUnder_  { false };
  OptCoord     fillUnderY_;
  double       angle_      { 0.0 };
  double       scale_      { 1.0 };
  Point2D      center_     { 0, 0 };
  Point2D      offset_     { 0, 0 };
  QPainterPath path_;
};

}

#endif
