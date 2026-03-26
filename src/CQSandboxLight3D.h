#ifndef CQSandboxLight3D_H
#define CQSandboxLight3D_H

#include <CGLVector3D.h>

#include <QObject>

class CQGLBuffer;

namespace CQSandbox {

class Canvas3D;
class ShaderProgram;

class Light3D : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool  enabled   READ isEnabled WRITE setEnabled)
  Q_PROPERTY(float intensity READ intensity WRITE setIntensity)
  Q_PROPERTY(float cutoff    READ cutoff    WRITE setCutoff)
  Q_PROPERTY(float radius    READ radius    WRITE setRadius)

 public:
  enum class Type {
    DIRECTIONAL,
    POINT,
    SPOT
  };

 public:
  Light3D(Canvas3D *canvas, const Type &type=Type::DIRECTIONAL);
 ~Light3D();

  int id() const { return id_; }
  void setId(int i) { id_ = i; }

  const Type &type() const { return type_; }
  void setType(const Type &t) { type_ = t; }

  bool isEnabled() const { return enabled_; }
  void setEnabled(bool b) { enabled_ = b; }

  const CGLVector3D &color() const { return color_; }
  void setColor(const CGLVector3D &c) { color_ = c; }

  // position (for directional light)
  const CGLVector3D &position() const { return position_; }
  void setPosition(const CGLVector3D &p) { position_ = p; }

  // direction (for point or spot)
  const CGLVector3D &direction() const { return direction_; }
  void setDirection(const CGLVector3D &p) { direction_ = p; }

  float intensity() const { return intensity_; }
  void setIntensity(float r) { intensity_ = r; }

  // cut off (for spot light)
  float cutoff() const { return cutoff_; }
  void setCutoff(float r) { cutoff_ = r; }

  // radius (for point light)
  float radius() const { return radius_; }
  void setRadius(float r) { radius_ = r; }

  void initBuffer();
  void initShader();

  void render();

 private:
  static ShaderProgram* s_program;

  Canvas3D* canvas_ { nullptr };

  int id_ { 0 };

  Type type_ { Type::DIRECTIONAL };

  bool enabled_ { false };

  CGLVector3D color_ { 1.0f, 1.0f, 1.0f };

  CGLVector3D position_  { 0.4f, 0.4f, 0.4f };
  CGLVector3D direction_ { 0.0f, 0.0f, 0.0f };

  CQGLBuffer* buffer_ { nullptr };

  float intensity_ { 1.0f };

  float cutoff_ { 0.1f };
  float radius_ { 0.1f };
};

}

#endif
