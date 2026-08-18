#ifndef CQSandboxLight3D_H
#define CQSandboxLight3D_H

#include <CGeomLight3D.h>

#include <QObject>

class CQGLBuffer;

namespace CQSandbox {

class Canvas3D;
class ShaderProgram;

class Light3D : public QObject, public CGeomLight3D {
  Q_OBJECT

  Q_PROPERTY(bool  enabled         READ getEnabled         WRITE setEnabled)
  Q_PROPERTY(float spotCutOffAngle READ getSpotCutOffAngle WRITE setSpotCutOffAngle)
  Q_PROPERTY(float pointRadius     READ getPointRadius     WRITE setPointRadius)

 public:
  Light3D(Canvas3D *canvas, const Type &type=Type::DIRECTIONAL);

  virtual ~Light3D();

  int id() const { return id_; }
  void setId(int i) { id_ = i; }

  void setType(const Type &t) override {
    CGeomLight3D::setType(t); notifyChanged(); }

  void setEnabled(bool b) override {
    CGeomLight3D::setEnabled(b); notifyChanged(); }

  void setDiffuse(const CRGBA &c) override {
    CGeomLight3D::setDiffuse(c); notifyChanged(); }

  // position (for directional light)
  void setPosition(const CPoint3D &p) override {
    CGeomLight3D::setPosition(p); notifyChanged(); }

  // direction (for point or spot)
  void setDirection(const CVector3D &d) override {
    CGeomLight3D::setDirection(d); notifyChanged(); }

  // cut off (for spot light)
  void setSpotCutOffAngle(double a) override {
    CGeomLight3D::setSpotCutOffAngle(a); notifyChanged(); }

  // radius (for point light)
  void setPointRadius(double r) override {
    CGeomLight3D::setPointRadius(r); notifyChanged(); }

  //---

  void initBuffer();
  void initShader();

  void render();

 Q_SIGNALS:
  void changedSignal();

 private:
  void notifyChanged();

 private:
  static ShaderProgram* s_program;

  Canvas3D*   canvas_ { nullptr };
  int         id_     { 0 };
  CQGLBuffer* buffer_ { nullptr };
};

}

#endif
