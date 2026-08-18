#ifndef CGLPath3D_H
#define CGLPath3D_H

#include <CVector3D.h>

class CGLPath3D {
 public:
  enum class ElementType {
    NONE,
    MOVE,
    LINE
  };

  struct Element {
    static Element makeElement(ElementType type, const CVector3D &p) {
      Element e;
      e.type = type; e.pos = p;
      return e;
    }

    ElementType type { ElementType::NONE };
    CVector3D   pos;
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

 private:
  Elements elements_;
};

#endif
