#include <CQSandboxObject3D.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxGroup3DObj.h>
#include <CQSandboxBBox3DObj.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

namespace CQSandbox {

Object3D::
Object3D(Canvas3D *canvas) :
 canvas_(canvas)
{
}

QString
Object3D::
getCommandName() const
{
  return QString("object3d.%1").arg(ind_);
}

QString
Object3D::
calcId() const
{
  auto id = this->id();

  if (id == "")
    id = getCommandName();

  return id;
}

void
Object3D::
setSelected(bool b)
{
  selected_ = b;

  setNeedsUpdate();
}

void
Object3D::
setXAngle(double a)
{
  setAngles(a, yAngle_, zAngle_);
}

void
Object3D::
setYAngle(double a)
{
  setAngles(xAngle_, a, zAngle_);
}

void
Object3D::
setZAngle(double a)
{
  setAngles(xAngle_, yAngle_, a);
}

void
Object3D::
setAngles(double xa, double ya, double za)
{
  xAngle_ = xa;
  yAngle_ = ya;
  zAngle_ = za;

  updateModelMatrix();

  setNeedsUpdate();
}

void
Object3D::
setXPos(double x)
{
  position_.x = x;

  setNeedsUpdate();
}

void
Object3D::
setYPos(double y)
{
  position_.y = y;

  setNeedsUpdate();
}

void
Object3D::
setZPos(double z)
{
  position_.z = z;

  setNeedsUpdate();
}

void
Object3D::
setScales(double xs, double ys, double zs)
{
  xscale_ = xs;
  yscale_ = ys;
  zscale_ = zs;

  setNeedsUpdate();
}

void
Object3D::
setPosition(const CPoint3D &p)
{
  position_ = p;

  updateModelMatrix();

  setNeedsUpdate();
}

CPoint3D
Object3D::
origin() const
{
  return origin_.value_or(position_);
}

void
Object3D::
setOrigin(const CPoint3D &p)
{
  origin_ = p;

  updateModelMatrix();

  setNeedsUpdate();
}

void
Object3D::
setNeedsUpdate()
{
  needsUpdate_ = true;
  bboxValid_   = false;
}

void
Object3D::
init()
{
  modelMatrix_ = CMatrix3DH::identity();
}

void
Object3D::
updateModelMatrix()
{
  setModelMatrix();
}

void
Object3D::
setModelMatrix(uint matrixFlags)
{
  // object centered at (0, 0). Moved to specified position
  auto o   = origin();
  auto pos = this->position();

  modelMatrix_ = CMatrix3DH::identity();

  if (matrixFlags & ModelMatrixFlags::ROTATE) {
    modelMatrix_.translated(float(o.getX()), float(o.getY()), float(o.getZ()));

    modelMatrix_.rotated(xAngle(), CVector3D(1.0, 0.0, 0.0));
    modelMatrix_.rotated(yAngle(), CVector3D(0.0, 1.0, 0.0));
    modelMatrix_.rotated(zAngle(), CVector3D(0.0, 0.0, 1.0));

    modelMatrix_.translated(-float(o.getX()), -float(o.getY()), -float(o.getZ()));
  }

  if (matrixFlags & ModelMatrixFlags::TRANSLATE)
    modelMatrix_.translated(float(pos.getX()), float(pos.getY()), float(pos.getZ()));

  if (matrixFlags & ModelMatrixFlags::SCALE)
    modelMatrix_.scaled(xscale(), yscale(), zscale());
}

bool
Object3D::
getValue(const QString &name, const QStringList &, QVariant &value)
{
  auto *app = canvas()->app();

  if      (name == "id")
    value = id();
  else if (name == "visible")
    value = QString(isVisible() ? "1" : "0");
  else if (name == "position")
    value = Util::point3DToString(position());
  else if (name == "x_angle")
    value = Util::realToString(Util::radToDeg(xAngle()));
  else if (name == "y_angle")
    value = Util::realToString(Util::radToDeg(yAngle()));
  else if (name == "z_angle")
    value = Util::realToString(Util::radToDeg(zAngle()));
  else if (name == "group")
    value = (group() ? group()->calcId() : "");
  else
    return app->errorMsg(QString("Invalid get name '%1'").arg(name));

  return true;
}

bool
Object3D::
setValue(const QString &name, const QString &value, const QStringList &)
{
  auto *app = canvas()->app();
  auto *tcl = app->tcl();

  if      (name == "id")
    setId(value);
  else if (name == "visible") {
    setVisible(Util::stringToBool(value));

    setNeedsUpdate();
  }
  else if (name == "position") {
    CPoint3D p;
    if (! Util::stringToPoint3D(tcl, value, p))
      return false;

    setPosition(p);

    setNeedsUpdate();
  }
  else if (name == "x_angle") {
    double a;
    if (! Util::stringToReal(value, a))
      return false;

    setXAngle(Util::degToRad(a));

    setNeedsUpdate();
  }
  else if (name == "y_angle") {
    double a;
    if (! Util::stringToReal(value, a))
      return false;

    setYAngle(Util::degToRad(a));

    setNeedsUpdate();
  }
  else if (name == "z_angle") {
    double a;
    if (! Util::stringToReal(value, a))
      return false;

    setZAngle(Util::degToRad(a));

    setNeedsUpdate();
  }
  else if (name == "scale") {
    double s;
    if (! Util::stringToReal(value, s))
      return false;

    setScale(s);

    setNeedsUpdate();
  }
  else if (name == "group") {
    auto *group = dynamic_cast<Group3DObj *>(canvas()->getObjectByName(value));

    if (group != group_) {
      if (group_)
        group_->removeObject(this);
      else
        canvas()->removeObject(this);

      if (group)
        group->addObject(this);
      else
        canvas()->addObject(this);
    }
  }
  else
    return app->errorMsg(QString("Invalid set name '%1'").arg(name));

  return true;
}

bool
Object3D::
exec(const QString &, const QStringList &, QVariant &)
{
  return false;
}

void
Object3D::
tick()
{
  auto *app = canvas()->app();

  ticks_ += dt_;

  elapsed_ += canvas_->redrawTimeOut()/1000.0;

  app->runTclCmd("tick");
}

void
Object3D::
render()
{
}

void
Object3D::
createBBoxObj()
{
  if (! bboxObj_) {
    bboxObj_ = new BBox3DObj(canvas_);

    bboxObj_->init();
  }

  bboxObj_->setPosition(bbox_.getCenter());

  bboxObj_->setScales(bbox_.getXSize(), bbox_.getYSize(), bbox_.getZSize());
}

}
