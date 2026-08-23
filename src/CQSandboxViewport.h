#ifndef CQSandboxViewport_H
#define CQSandboxViewport_H

namespace CQSandbox {

class Canvas;

struct Viewport {
  using Objects = std::vector<Object *>;

  Canvas*         canvas { nullptr };
  QString         name;
  CDisplayRange2D displayRange;
  Rect            rect;
  QPen            pen;
  AnimateBrush    brush;
  Objects         objects;
  Rect            clip;
  bool            hasRange { false };
};

}

#endif
