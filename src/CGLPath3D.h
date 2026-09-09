#ifndef CGLPath3D_H
#define CGLPath3D_H

#include <CVector3D.h>

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
    using Points = std::vector<CVector3D>;

    static Element makeElement(ElementType type, const CVector3D &p) {
      Points points = { p };
      return makeElement(type, points);
    }

    static Element makeElement(ElementType type, const CVector3D &p1, const CVector3D &p2) {
      Points points = { p1, p2 };
      return makeElement(type, points);
    }

    static Element makeElement(ElementType type, const CVector3D &p1,
                               const CVector3D &p2, const CVector3D &p3) {
      Points points = { p1, p2, p3 };
      return makeElement(type, points);
    }

    static Element makeElement(ElementType type, const Points &points) {
      Element e;
      e.type = type; e.points = points;
      return e;
    }

    ElementType            type { ElementType::NONE };
    std::vector<CVector3D> points;
  };

  using Elements = std::vector<Element>;

 public:
  CGLPath3D() { }

  const Elements &elements() const { return elements_; }

  void clear() {
    elements_.clear();
  }

  void moveTo(const CVector3D &p) {
    elements_.push_back(Element::makeElement(ElementType::MOVE, p));
  }

  void lineTo(const CVector3D &p) {
    elements_.push_back(Element::makeElement(ElementType::LINE, p));
  }

  void quadTo(const CVector3D &p1, const CVector3D &p2) {
    elements_.push_back(Element::makeElement(ElementType::QUAD, p1, p2));
  }

  void cubicTo(const CVector3D &p1, const CVector3D &p2, const CVector3D &p3) {
    elements_.push_back(Element::makeElement(ElementType::CUBIC, p1, p2, p3));
  }

  bool calc(double t, CPoint3D &pi) {
    auto vectorDistance = [](const CVector3D &v1, const CVector3D &v2) {
      return v1.point().distanceTo(v2.point());
    };

    double len = 0.0;

    CVector3D p;

    for (const auto &element : elements_) {
      if      (element.type == ElementType::MOVE) {
        p = element.points[0];
      }
      else if (element.type == ElementType::LINE) {
        auto p1 = element.points[0];

        len += vectorDistance(p, p1);

        p = p1;
      }
      else if (element.type == ElementType::QUAD) {
        auto p1 = element.points[1];

        len += vectorDistance(p, p1);

        p = p1;
      }
      else if (element.type == ElementType::CUBIC) {
        auto p1 = element.points[2];

        len += vectorDistance(p, p1);

        p = p1;
      }
    }

    auto tlen = t*len;

    len = 0.0;

    for (const auto &element : elements_) {
      if      (element.type == ElementType::MOVE) {
        p = element.points[0];
      }
      else if (element.type == ElementType::LINE) {
        auto p1 = element.points[0];

        auto len1 = len + vectorDistance(p, p1);

        if (tlen >= len && tlen <= len1) {
          auto t1 = (tlen - len)/(len1 - len);

          pi = ((p1 - p)*t1 + p1).point();

          return true;
        }

        p   = p1;
        len = len1;
      }
      else if (element.type == ElementType::QUAD) {
        auto p1 = element.points[1];

        auto len1 = len + vectorDistance(p, p1);

        if (tlen >= len && tlen <= len1) {
          auto t1 = (tlen - len)/(len1 - len);

          pi = ((p1 - p)*t1 + p1).point();

          return true;
        }

        p   = p1;
        len = len1;
      }
      else if (element.type == ElementType::CUBIC) {
        auto p1 = element.points[2];

        auto len1 = len + vectorDistance(p, p1);

        if (tlen >= len && tlen <= len1) {
          auto t1 = (tlen - len)/(len1 - len);

          pi = ((p1 - p)*t1 + p1).point();

          return true;
        }

        p   = p1;
        len = len1;
      }
    }

    return false;
  }

 private:
  Elements elements_;
};

#endif
