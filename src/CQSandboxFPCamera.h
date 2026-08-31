#ifndef FPCamera_H
#define FPCamera_H

#include <CQSandboxCamera.h>

namespace CQSandbox {

class App;

class FPCamera : public CameraIFace {
  Q_OBJECT

 public:
  FPCamera(App *app);

  //---

  // origin (for rotation center)
  const CVector3D &origin() const override;
  void setOrigin(const CVector3D &p) override;

  // position (for camera position)
  const CVector3D &position() const override;
  void setPosition(const CVector3D &p) override;

  void movePosition(const CVector3D &p);

  //---

  // rotation x angle
  double pitch() const override;
  void setPitch(double r) override;

  // rotation x angle
  double yaw() const override;
  void setYaw(double r) override;

  //---

  // coordinate system vectors
  CVector3D front() const override;
  CVector3D up   () const override;
  CVector3D right() const override;

  //---

  void reset(const CBBox3D &bbox) override;

  //---

  void moveFront(double d) override;
  void moveUp   (double d) override;
  void moveRight(double d) override;

  void rotateX(double da) override; // pitch
  void rotateY(double da) override; // yaw

  CMatrix3DH perspectiveMatrix() const override;
  CMatrix3DH orthoMatrix() const override;
  CMatrix3DH viewMatrix() const override;

  double distance() const override { return distance_; }
  void setDistance(double r) override { if (distance_ != r) { distance_ = r; stateChanged(); } }

  //---

  void getPixelRay(double x, double y, CPoint3D &rp1, CPoint3D &rp2) const;

  //---

  void stateChanged() override { orientationValid_ = false; Q_EMIT stateChangedSignal(); }

  //---

  void printMatrices() const;

 private:
  void updateOrientation() const;
  void updateOrientationI();

  void calcViewMatrix();

  void calcPerspectiveMatrix();
  void calcOrthoMatrix();

  CVector3D toEuler(const CQuaternion &q) const;

  void lookAt(const CVector3D &forward, const CVector3D &up);

  CQuaternion calcLookAt(const CVector3D &forward, const CVector3D &up) const;

 private:
  CQuaternion orientation_;
  bool        orientationValid_ { false };

  CVector3D front_ { 0, 0, 1 };
  CVector3D right_ { 1, 0, 0 };

  CMatrix3DH perspectiveMatrix_ { CMatrix3DH::identity() };
  CMatrix3DH orthoMatrix_       { CMatrix3DH::identity() };
  CMatrix3DH viewMatrix_        { CMatrix3DH::identity() };

  CVector3D originDelta_ { 0, 0, 0 };
  CVector3D angleDelta_  { 0, 0, 0 };

  bool originChanged_ { false };
  bool angleChanged_  { false };

  double distance_ { 1.0 };
};

}

#endif
