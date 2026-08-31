#ifndef Camera_H
#define Camera_H

#include <CGLCameraIFace.h>
#include <CQuaternion.h>
#include <CVector3D.h>
#include <CBBox3D.h>

#include <QObject>

namespace CQSandbox {

class App;

class CameraIFace : public QObject, public CGLCameraIFace {
  Q_OBJECT

 public:
  CameraIFace(App *app) :
   app_(app) {
  }

  //---

  bool isClampPitch() const { return clampPitch_.enabled; }
  void setClampPitch(bool b) { clampPitch_.enabled = b; stateChanged(); }

  double minPitch() const { return clampPitch_.min; }
  void setMinPitch(double r) { clampPitch_.min = r; stateChanged(); }

  double maxPitch() const { return clampPitch_.max; }
  void setMaxPitch(double r) { clampPitch_.max = r; stateChanged(); }

  //---

  bool isClampYaw() const { return clampYaw_.enabled; }
  void setClampYaw(bool b) { clampYaw_.enabled = b; stateChanged(); }

  double minYaw() const { return clampYaw_.min; }
  void setMinYaw(double r) { clampYaw_.min = r; stateChanged(); }

  double maxYaw() const { return clampYaw_.max; }
  void setMaxYaw(double r) { clampYaw_.max = r; stateChanged(); }

  //---

  bool isClampRoll() const { return clampRoll_.enabled; }
  void setClampRoll(bool b) { clampRoll_.enabled = b; stateChanged(); }

  double minRoll() const { return clampRoll_.min; }
  void setMinRoll(double r) { clampRoll_.min = r; stateChanged(); }

  double maxRoll() const { return clampRoll_.max; }
  void setMaxRoll(double r) { clampRoll_.max = r; stateChanged(); }

  //---

  virtual double distance() const = 0;
  virtual void setDistance(double r) = 0;

  //---

  virtual void reset(const CBBox3D &bbox) = 0;

  virtual void zoomIn(const CBBox3D &bbox) {
    auto d = bbox.getMaxSize()/100.0;
    moveUp(d);
  }

  virtual void zoomOut(const CBBox3D &bbox) {
    auto d = bbox.getMaxSize()/100.0;
    moveUp(-d);
  }

 Q_SIGNALS:
  void stateChangedSignal();

 protected:
  App* app_ { nullptr };

  struct ClampData {
    bool   enabled { false };
    double min     { -M_PI/2.0 };
    double max     { -M_PI/2.0 };

    double clamp(double r) const {
      if (r < min) return min;
      if (r > max) return max;
      return r;
    }
  };

  ClampData clampPitch_;
  ClampData clampYaw_;
  ClampData clampRoll_;
};

}

//---

namespace CQSandbox {

class Camera : public CameraIFace {
  Q_OBJECT

 public:
  Camera(App *app);

  //---

  bool isDisableRoll() const { return disableRoll_; }
  void setDisableRoll(bool b) { disableRoll_ = b; stateChanged(); }

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

  // rotation y angle
  double yaw() const override;
  void setYaw(double r) override;

  // rotation z angle
  double roll() const override;
  void setRoll(double r) override;

  //---

  void reset(const CBBox3D &bbox) override;

  //---

  // coordinate system vectors
  CVector3D front() const override;
  CVector3D up   () const override;
  CVector3D right() const override;

  //---

  void moveFront(double d) override;
  void moveUp   (double d) override;
  void moveRight(double d) override;

  void rotateX(double da) override; // pitch
  void rotateY(double da) override; // yaw
  void rotateZ(double da) override; // roll

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
  bool disableRoll_ { false };

  CQuaternion orientation_;
  bool        orientationValid_ { false };

  CVector3D front_ { 0, 0, 1 };
  CVector3D up_    { 0, 1, 0 };
  CVector3D right_ { 1, 0, 0 };

  CMatrix3DH perspectiveMatrix_ { CMatrix3DH::identity() };
  CMatrix3DH orthoMatrix_       { CMatrix3DH::identity() };
  CMatrix3DH viewMatrix_        { CMatrix3DH::identity() };

  CVector3D originDelta_ { 0, 0, 0 };
  CVector3D angleDelta_  { 0, 0, 0 };

  double distance_ { 1.0 };
};

}

#endif
