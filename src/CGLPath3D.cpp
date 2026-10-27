#include <CGLPath3D.h>

#include <CSVGUtil.h>

CGLPath3D::
CGLPath3D()
{
}

bool
CGLPath3D::
calc(double t, uint numLines, CPoint3D &pi, CVector3D &dir) const
{
  updateLines(numLines);

  auto pointDistance = [](const CPoint3D &p1, const CPoint3D &p2) {
    return p1.distanceTo(p2);
  };

  double len = 0.0;

  for (const auto &line : lines_) {
    const auto &p1 = line.start();
    const auto &p2 = line.end  ();

    len += pointDistance(p1, p2);
  }

  auto tlen = t*len;

  len = 0.0;

  for (const auto &line : lines_) {
    const auto &p1 = line.start();
    const auto &p2 = line.end  ();

    auto len1 = len + pointDistance(p1, p2);

    if (tlen >= len && tlen <= len1) {
      auto t1 = (tlen - len)/(len1 - len);

      pi = (p2 - p1)*t1 + p1;

      dir = CVector3D(p1, p2);

      return true;
    }

    len = len1;
  }

  return false;
}

void
CGLPath3D::
toLines(uint numLines, std::vector<CLine3D> &lines) const
{
  lines.clear();

  CPoint3D lastPoint;

  for (const auto &element : elements_) {
    if      (element.type == CGLPath3D::ElementType::MOVE) {
      lastPoint = element.points[0];
    }
    else if (element.type == CGLPath3D::ElementType::LINE) {
      auto p = element.points[0];

      lines.push_back(CLine3D(lastPoint, p));

      lastPoint = p;
    }
    else if (element.type == CGLPath3D::ElementType::QUAD) {
      auto p1 = element.points[0];
      auto p2 = element.points[1];

      C2Bezier3D quad(lastPoint, p1, p2);

      auto dt = 1.0/numLines;

      auto t = dt;

      for (uint i = 0; i < numLines; ++i) {
        auto pt = quad.calc(t);

        lines.push_back(CLine3D(lastPoint, pt));

        t += dt;

        lastPoint = pt;
      }
    }
    else if (element.type == CGLPath3D::ElementType::CUBIC) {
      auto p1 = element.points[0];
      auto p2 = element.points[1];
      auto p3 = element.points[2];

      C3Bezier3D cubic(lastPoint, p1, p2, p3);

      auto dt = 1.0/numLines;

      auto t = dt;

      for (uint i = 0; i < numLines; ++i) {
        auto pt = cubic.calc(t);

        lines.push_back(CLine3D(lastPoint, pt));

        t += dt;

        lastPoint = pt;
      }
    }
  }
}

bool
CGLPath3D::
fromSvg(const std::string &str)
{
  class PathVisitor : public CSVGUtil::PathVisitor {
   public:
    PathVisitor() { }

    const CGLPath3D &path() const { return path_; }

    void moveTo(double x, double y) override {
      path_.moveTo(point3d(x, y));
    }

    void lineTo(double x, double y) override {
      path_.lineTo(point3d(x, y));
    }

    void arcTo(double rx, double ry, double xa, int fa, int fs, double x2, double y2) override {
      bool unit_circle = false;

      //double cx, cy, rx1, ry1, theta, delta;

      //CSVGUtil::convertArcCoords(lastX(), lastY(), x2, y2, xa, rx, ry, fa, fs, unit_circle,
      //                           &cx, &cy, &rx1, &ry1, &theta, &delta);

      //path_.arcTo(QRectF(cx - rx1, cy - ry1, 2*rx1, 2*ry1), -theta, -delta);

      //double a1 = CMathUtil::Deg2Rad(theta);
      //double a2 = CMathUtil::Deg2Rad(theta + delta);

      CSVGUtil::BezierList beziers;

      CSVGUtil::arcToBeziers(lastX(), lastY(), x2, y2, xa, rx, ry, fa, fs, unit_circle, beziers);

      if (! beziers.empty())
        path_.lineTo(point3d(beziers[0].getFirstPoint()));

      for (const auto &bezier : beziers) {
        path_.cubicTo(point3d(bezier.getControlPoint1()),
                      point3d(bezier.getControlPoint2()),
                      point3d(bezier.getLastPoint    ()));
      }
    }

    void bezier2To(double x1, double y1, double x2, double y2) override {
      path_.quadTo(point3d(x1, y1), point3d(x2, y2));
    }

    void bezier3To(double x1, double y1, double x2, double y2, double x3, double y3) override {
      path_.cubicTo(point3d(x1, y1), point3d(x2, y2), point3d(x3, y3));
    }

    void closePath(bool /*relative*/) override {
      path_.close();
    }

   private:
    CPoint3D point3d(double x, double y) { return point3d(CPoint2D(x, y)); };

    CPoint3D point3d(const CPoint2D &p) { return CPoint3D(p.x, 0, p.y); };

   private:
    CGLPath3D path_;
  };

  PathVisitor visitor;

  if (! CSVGUtil::visitPath(str, visitor)) {
    //std::cerr << "Invalid path: " << str.toStdString() << "\n";
    return false;
  }

  *this = visitor.path();

  return true;
}
