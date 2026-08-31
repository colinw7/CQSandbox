#include <CQSandboxOrthoCamera.h>
#include <CQSandboxApp.h>
#include <CQSandboxCanvas3D.h>

#include <cmath>

namespace CQSandbox {

static CVector3D CAMERA_WORLD_FORWARD = CVector3D(0.0, 0.0, -1.0);
static CVector3D CAMERA_WORLD_UP      = CVector3D(0.0, 1.0,  0.0);
static CVector3D CAMERA_WORLD_RIGHT   = CVector3D(1.0, 0.0,  0.0);

OrthoCamera::
OrthoCamera(App *app) :
 CameraIFace(app)
{
  setOrthoType(OthroType::TOP);
}

//---

void
OrthoCamera::
setOrthoType(const OthroType &t)
{
  orthoType_ = t;

  auto setPitchI = [&](double r) { pitch_ = r; };
  auto setYawI   = [&](double r) { yaw_   = r; };

  if      (orthoType_ == OthroType::TOP) {
    setPitchI(-M_PI/2.0);
    setYawI  (M_PI/2.0);

    front_ = CVector3D( 0.0, -1.0, 0.0);
    right_ = CVector3D(-1.0,  0.0, 0.0);
    up_    = CVector3D( 0.0,  0.0, 1.0);
  }
  else if (orthoType_ == OthroType::BOTTOM) {
    setPitchI(M_PI/2.0);
    setYawI  (M_PI/2.0);

    front_ = CVector3D(0.0, 1.0, 0.0);
    right_ = CVector3D(1.0, 0.0, 0.0);
    up_    = CVector3D(0.0, 0.0, 1.0);
  }
  else if (orthoType_ == OthroType::FRONT) {
    setPitchI(0.0);
    setYawI  (0.0);

    front_ = CVector3D(0.0, 0.0, 1.0);
    right_ = CVector3D(1.0, 0.0, 0.0);
    up_    = CVector3D(0.0, 1.0, 0.0);
  }
  else if (orthoType_ == OthroType::BACK) {
    setPitchI(M_PI);
    setYawI  (0.0);

    front_ = CVector3D( 0.0, 0.0, -1.0);
    right_ = CVector3D(-1.0, 0.0,  0.0);
    up_    = CVector3D( 0.0, 1.0,  0.0);
  }
  else if (orthoType_ == OthroType::LEFT) {
    setPitchI(0.0);
    setYawI  (M_PI/2.0);

    front_ = CVector3D(1.0, 0.0, 0.0);
    right_ = CVector3D(0.0, 0.0, 1.0);
    up_    = CVector3D(0.0, 1.0, 0.0);
  }
  else if (orthoType_ == OthroType::RIGHT) {
    setPitchI(0.0);
    setYawI  (-M_PI/2.0);

    front_ = CVector3D(-1.0, 0.0,  0.0);
    right_ = CVector3D( 0.0, 0.0, -1.0);
    up_    = CVector3D( 0.0, 1.0,  0.0);
  }

  roll_ = 0.0;

  //up_ = right_.crossProduct(front_).normalized();

  position_ = origin_ - front_*distance_;

  orientationValid_ = false;

  stateChanged();
}

//---

const CVector3D &
OrthoCamera::
origin() const
{
  return origin_;
}

void
OrthoCamera::
setOrigin(const CVector3D &p)
{
  origin_   = p;
  position_ = origin_ - front_*distance_;

  stateChanged();
}

const CVector3D &
OrthoCamera::
position() const
{
  return position_;
}

void
OrthoCamera::
setPosition(const CVector3D &p)
{
  position_ = p;
  origin_   = position_ + front_*distance_;

  stateChanged();
}

void
OrthoCamera::
setDistance(double r)
{
  distance_ = r;
  position_ = origin_ - front_*distance_;

  stateChanged();
}

//---

double
OrthoCamera::
pitch() const
{
  return pitch_;
}

double
OrthoCamera::
yaw() const
{
  return yaw_;
}

double
OrthoCamera::
roll() const
{
  return 0;
}

//---

CVector3D
OrthoCamera::
front() const
{
  return front_;
}

CVector3D
OrthoCamera::
up() const
{
  return up_;
}

CVector3D
OrthoCamera::
right() const
{
  return right_;
}

//---

void
OrthoCamera::
reset(const CBBox3D &bbox)
{
  auto center  = bbox.getCenter();
  auto maxSize = bbox.getMaxSize();

  origin_ = center;

  auto s2 = std::sqrt(2.0);

  distance_ = s2*maxSize + near();

  stateChanged();
}

//---

void
OrthoCamera::
updateOrientation() const
{
  if (orientationValid_)
    return;

  const_cast<OrthoCamera *>(this)->updateOrientationI();
}

void
OrthoCamera::
updateOrientationI()
{
  calcOrthoMatrix();
  calcPerspectiveMatrix();

  calcViewMatrix();
}

CMatrix3DH
OrthoCamera::
perspectiveMatrix() const
{
  updateOrientation();

  return perspectiveMatrix_;
}

void
OrthoCamera::
calcPerspectiveMatrix()
{
  perspectiveMatrix_ = CMatrix3DH::perspective(fov(), aspect(), near(), far());
}

CMatrix3DH
OrthoCamera::
orthoMatrix() const
{
  updateOrientation();

  return orthoMatrix_;
}

void
OrthoCamera::
calcOrthoMatrix()
{
  auto *canvas = app_->canvas3D();
  if (! canvas) return;

  const auto &bbox = canvas->bbox();

  orthoMatrix_ = CMatrix3DH::ortho(bbox.getXMin(), bbox.getXMax(), bbox.getYMin(), bbox.getYMax(),
                                   near(), far());
}

CMatrix3DH
OrthoCamera::
viewMatrix() const
{
  updateOrientation();

  return viewMatrix_;
}

void
OrthoCamera::
calcViewMatrix()
{
  viewMatrix_.setLookAt(position().point(), front(), up(), right());
}

}
