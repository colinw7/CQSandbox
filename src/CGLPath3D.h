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
  CGLPath3D();

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

  void close() { }

  bool calc(double t, uint numLines, CPoint3D &pi, CVector3D &dir) const;

  void toLines(uint numLines, std::vector<CLine3D> &lines) const;

  bool fromSvg(const std::string &str);

 private:
  void updateLines(uint numLines) const {
    if (! linesValid_ || numLines != numLines_) {
      auto *th = const_cast<CGLPath3D *>(this);

      th->numLines_ = numLines;

      toLines(th->numLines_, th->lines_);
    }
  }

 private:
  Elements elements_;

  bool                 linesValid_ { false };
  uint                 numLines_   { 0 };
  std::vector<CLine3D> lines_;
};

#endif
