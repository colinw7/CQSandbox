#ifndef CGLPath3D_H
#define CGLPath3D_H

#include <CPoint3D.h>
#include <CLine3D.h>
#include <C2Bezier3D.h>
#include <C3Bezier3D.h>

class CGLPath3D {
 public:
  enum class ElementType {
    NONE,
    MOVE,
    LINE,
    QUAD,
    CUBIC
  };

  struct Element {
    using Points = std::vector<CPoint3D>;

    static Element makeElement(ElementType type, const CPoint3D &p) {
      Points points = { p };
      return makeElement(type, points);
    }

    static Element makeElement(ElementType type, const CPoint3D &p1, const CPoint3D &p2) {
      Points points = { p1, p2 };
      return makeElement(type, points);
    }

    static Element makeElement(ElementType type, const CPoint3D &p1,
                               const CPoint3D &p2, const CPoint3D &p3) {
      Points points = { p1, p2, p3 };
      return makeElement(type, points);
    }

    static Element makeElement(ElementType type, const Points &points) {
      Element e;
      e.type = type; e.points = points;
      return e;
    }

    ElementType           type { ElementType::NONE };
    std::vector<CPoint3D> points;
  };

  using Elements = std::vector<Element>;

 public:
  CGLPath3D() { }

  const Elements &elements() const { return elements_; }

  void clear() {
    elements_.clear();

    linesValid_ = false;
  }

  void moveTo(const CPoint3D &p) {
    elements_.push_back(Element::makeElement(ElementType::MOVE, p));

    linesValid_ = false;
  }

  void lineTo(const CPoint3D &p) {
    elements_.push_back(Element::makeElement(ElementType::LINE, p));

    linesValid_ = false;
  }

  void quadTo(const CPoint3D &p1, const CPoint3D &p2) {
    elements_.push_back(Element::makeElement(ElementType::QUAD, p1, p2));

    linesValid_ = false;
  }

  void cubicTo(const CPoint3D &p1, const CPoint3D &p2, const CPoint3D &p3) {
    elements_.push_back(Element::makeElement(ElementType::CUBIC, p1, p2, p3));

    linesValid_ = false;
  }

  bool calc(double t, uint numLines, CPoint3D &pi) {
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

        return true;
      }

      len = len1;
    }

    return false;
  }

  void toLines(uint numLines, std::vector<CLine3D> &lines) {
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

 private:
  void updateLines(uint numLines) {
    if (! linesValid_ || numLines != numLines_) {
      numLines_ = numLines;

      toLines(numLines_, lines_);
    }
  }

 private:
  Elements elements_;

  bool                 linesValid_ { false };
  uint                 numLines_   { 0 };
  std::vector<CLine3D> lines_;
};

#endif
