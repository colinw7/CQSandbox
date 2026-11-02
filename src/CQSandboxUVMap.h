#ifndef CQCamera3DUVMap_H
#define CQCamera3DUVMap_H

#include <CPoint2D.h>

#include <QFrame>

namespace CQSandbox {

class Canvas3D;
class Object3D;

class UVMap : public QFrame {
  Q_OBJECT

  Q_PROPERTY(bool wrapValues READ isWrapValues WRITE setWrapValues)

 public:
  enum class TextureType {
    NONE,
    DIFFUSE,
    NORMAL,
    SPECULAR,
    EMISSIVE
  };

 public:
  UVMap(Canvas3D *canvas);

  const Object3D *object() const { return object_; }
  void setObject(Object3D *o) { object_ = o; update(); }

  const TextureType &textureType() const { return textureType_; }
  void setTextureType(const TextureType &t) { textureType_ = t; update(); }

  bool isWrapValues() const { return wrapValues_; }
  void setWrapValues(bool b) { wrapValues_ = b; update(); }

  void paintEvent(QPaintEvent *) override;

  void mousePressEvent(QMouseEvent *) override;
  void mouseMoveEvent (QMouseEvent *) override;

 private:
  double mapU(double u) { return (w_ - 1)*(u - umin_)/(umax_ - umin_); }
  double mapV(double v) { return (h_ - 1)*(v - vmin_)/(vmax_ - vmin_); }

  double unmapU(double u) { return u*(umax_ - umin_)/(w_ - 1) + umin_; }
  double unmapV(double v) { return v*(vmax_ - vmin_)/(h_ - 1) + vmin_; }

 private:
  Canvas3D*   canvas_      { nullptr };
  Object3D*   object_      { nullptr };
  TextureType textureType_ { TextureType::DIFFUSE };
  QColor      bgColor_     { 0, 0, 0 };
  QColor      lineColor_   { 255, 255, 255 };
  bool        wrapValues_  { false };

  mutable double umin_ { 0.0 };
  mutable double umax_ { 1.0 };
  mutable double vmin_ { 0.0 };
  mutable double vmax_ { 1.0 };

  mutable int w_ { 100 };
  mutable int h_ { 100 };

  struct TexturePoint {
    CPoint2D tpoint;
    QPointF  p;
  };

  std::vector<TexturePoint> texturePoints_;

#if 0
  CGeomFace3D* selectedFace_ { nullptr };
  int          selectedInd_  { -1 };
#endif
};

}

#endif
