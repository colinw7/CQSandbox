#include <CQSandboxGroup3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxBBox3DObj.h>
#include <CQSandboxApp.h>

#include <CQTclUtil.h>

namespace CQSandbox {

bool
Group3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Group3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Group3DObj::
Group3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

void
Group3DObj::
init()
{
  Object3D::init();
}

QVariant
Group3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

bool
Group3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  return Object3D::setValue(name, value, args);
}

void
Group3DObj::
addObject(Object3D *obj)
{
  objects_.push_back(obj);

  obj->setGroup(this);

  bboxValid_ = false;

  Q_EMIT objectsChanged();
}

void
Group3DObj::
removeObject(Object3D *obj)
{
  Objects objects;

  for (auto *obj1 : objects_) {
    if (obj1 != obj)
      objects.push_back(obj1);
  }

  std::swap(objects, objects_);

  bboxValid_ = false;

  Q_EMIT objectsChanged();
}

void
Group3DObj::
setModelMatrix(uint matrixFlags)
{
  auto bbox = calcBBox();

  auto c = bbox.getCenter();

  modelMatrix_ = CGLMatrix3D::identity();

  if (matrixFlags & ModelMatrixFlags::TRANSLATE)
    modelMatrix_.translated(c.x + float(position().getX()),
                            c.y + float(position().getY()),
                            c.z + float(position().getZ()));
  else
    modelMatrix_.translated(c.x, c.y, c.z);

  if (matrixFlags & ModelMatrixFlags::SCALE)
    modelMatrix_.scaled(xscale(), yscale(), zscale());

  if (matrixFlags & ModelMatrixFlags::ROTATE) {
    modelMatrix_.rotated(xAngle(), CGLVector3D(1.0, 0.0, 0.0));
    modelMatrix_.rotated(yAngle(), CGLVector3D(0.0, 1.0, 0.0));
    modelMatrix_.rotated(zAngle(), CGLVector3D(0.0, 0.0, 1.0));
  }

  modelMatrix_.translated(-c.x, -c.y, -c.z);
}

void
Group3DObj::
render()
{
  for (auto *obj : objects_) {
    if (! obj || ! obj->isVisible())
      continue;

    obj->render();
  }

  //---

  if (canvas_->isBBox() || isSelected()) {
    calcBBox();

    createBBoxObj();

    bboxObj_->render();
  }
}

void
Group3DObj::
initOrigin()
{
  bboxValid_ = false;

  calcBBox();

  auto o = bbox_.getCenter();

  for (auto *obj : objects_)
    obj->setOrigin(o);
}

CPoint3D
Group3DObj::
origin() const
{
  return bbox_.getCenter();
}

void
Group3DObj::
setAngles(double xa, double ya, double za)
{
  xAngle_ = xa;
  yAngle_ = ya;
  zAngle_ = za;

  for (auto *obj : objects_)
    obj->setAngles(xa, ya, za);

  bboxValid_ = false;
}

CBBox3D
Group3DObj::
calcBBox()
{
  if (! bboxValid_) {
    bbox_ = CBBox3D();

    for (auto *obj : objects_)
      bbox_ += obj->calcBBox();

    bboxValid_ = true;
  }

  return bbox_;
}

}
