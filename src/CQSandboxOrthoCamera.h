#ifndef OrthoCamera_H
#define OrthoCamera_H

#include <CQSandboxCamera.h>

namespace CQSandbox {

class App;

class OrthoCamera : public CameraIFace {
  Q_OBJECT

 public:
  enum class OthroType {
    TOP,
    BOTTOM,
    LEFT,
    RIGHT,
    FRONT,
    BACK
  };

 public:
  OrthoCamera(App *app);

  //---

  const OthroType &orthoType() const { return orthoType_; }
  void setOrthoType(const OthroType &v);

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

  // rotation y angle
  double yaw() const override;

  // rotation z angle
  double roll() const override;

  //---

  // coordinate system vectors
  CVector3D front() const override;
  CVector3D up   () const override;
  CVector3D right() const override;

  //---

  double distance() const override { return distance_; }
  void setDistance(double r) override;

  //---

  void reset(const CBBox3D &bbox) override;

  void zoomIn(const CBBox3D &bbox) override {
    auto d = bbox.getMaxSize()/100.0;
    setDistance(distance() - d);
  }

  void zoomOut(const CBBox3D &bbox) override {
    auto d = bbox.getMaxSize()/100.0;
    setDistance(distance() + d);
  }

  //---

  CMatrix3DH perspectiveMatrix() const override;
  CMatrix3DH orthoMatrix() const override;
  CMatrix3DH viewMatrix() const override;

  //---

  void stateChanged() override { orientationValid_ = false; Q_EMIT stateChangedSignal(); }

 Q_SIGNALS:
  void stateChangedSignal();

 private:
  void updateOrientation() const;
  void updateOrientationI();

  void calcViewMatrix();

  void calcPerspectiveMatrix();
  void calcOrthoMatrix();

 private:
  OthroType orthoType_ { OthroType::TOP };

  CVector3D front_ { 0, 0, 1 };
  CVector3D right_ { 1, 0, 0 };
  CVector3D up_    { 0, 1, 0 };

  bool orientationValid_ { false };

  CMatrix3DH perspectiveMatrix_ { CMatrix3DH::identity() };
  CMatrix3DH orthoMatrix_       { CMatrix3DH::identity() };
  CMatrix3DH viewMatrix_        { CMatrix3DH::identity() };

  double distance_ { 1.0 };
};

}

#endif
