#ifndef CQSandboxLight3D_H
#define CQSandboxLight3D_H

#include <CGeomLight3D.h>
#include <CQSandboxCamera.h>

#include <QObject>

class CQGLBuffer;

namespace CQSandbox {

class Canvas3D;
class ShaderProgram;

class Light3D : public CameraIFace, public CGeomLight3D {
  Q_OBJECT

  Q_PROPERTY(bool  enabled         READ getEnabled         WRITE setEnabled)
  Q_PROPERTY(float spotCutOffAngle READ getSpotCutOffAngle WRITE setSpotCutOffAngle)
  Q_PROPERTY(float pointRadius     READ getPointRadius     WRITE setPointRadius)
  Q_PROPERTY(float power           READ getPower           WRITE setPower)

 public:
  Light3D(Canvas3D *canvas, const Type &type=Type::DIRECTIONAL);

  virtual ~Light3D();

  //---

  int id() const { return id_; }
  void setId(int i) { id_ = i; notifyChanged(); }

  void setName(const std::string &name);

  //---

  const CVector3D &position() const override;
  void setPosition(const CVector3D &p) override;

  const CVector3D &getDirection() const override;
  void setDirection(const CVector3D &dir) override;

  //--

  void reset(const CBBox3D &bbox) override;

  // coordinate system vectors
  CVector3D front() const override;
  CVector3D up   () const override;
  CVector3D right() const override;

  CMatrix3DH perspectiveMatrix() const override;
  CMatrix3DH orthoMatrix() const override;
  CMatrix3DH viewMatrix() const override;

  double distance() const override;
  void setDistance(double r) override;

  //---

  void initBuffer();
  void initShader();

  void render();

  CQGLBuffer *getBuffer() const { return buffer_; }

 Q_SIGNALS:
  void changedSignal();

 private:
  void notifyChanged() override;

 private:
  static ShaderProgram* s_program;

  Canvas3D*   canvas_ { nullptr };
  int         id_     { 0 };
  CQGLBuffer* buffer_ { nullptr };

  mutable CVector3D position_;

  CVector3D front_ { 0, 0, 1 };
  CVector3D right_ { 1, 0, 0 };
  CVector3D up_    { 0, 1, 0 };
};

}

#endif
