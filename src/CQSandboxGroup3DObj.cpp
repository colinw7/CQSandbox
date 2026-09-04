#include <CQSandboxGroup3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxBBox3DObj.h>
#include <CQSandboxModel3DObj.h>
#include <CQSandboxGeomObject.h>
#include <CQSandboxApp.h>

#include <CQTclUtil.h>

namespace CQSandbox {

Object3D *
Group3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->tcl();

  auto *obj = new Group3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

Group3DObj::
Group3DObj(Canvas3D *canvas, const Type &type) :
 Object3D(canvas, type)
{
}

void
Group3DObj::
init()
{
  Object3D::init();
}

bool
Group3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  return Object3D::getValue(name, args, value);
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

  modelMatrix_ = CMatrix3DH::identity();

  if (matrixFlags & ModelMatrixFlags::TRANSLATE)
    modelMatrix_.translated(c.x + float(position().getX()),
                            c.y + float(position().getY()),
                            c.z + float(position().getZ()));
  else
    modelMatrix_.translated(c.x, c.y, c.z);

  if (matrixFlags & ModelMatrixFlags::SCALE)
    modelMatrix_.scaled(xScale(), yScale(), zScale());

  if (matrixFlags & ModelMatrixFlags::ROTATE) {
    modelMatrix_.rotated(xAngle(), CVector3D(1.0, 0.0, 0.0));
    modelMatrix_.rotated(yAngle(), CVector3D(0.0, 1.0, 0.0));
    modelMatrix_.rotated(zAngle(), CVector3D(0.0, 0.0, 1.0));
  }

  modelMatrix_.translated(-c.x, -c.y, -c.z);
}

void
Group3DObj::
render()
{
#if 0
  bool        singleBuffer = true;
  CQGLBuffer* buffer       = nullptr;

  for (auto *obj : objects_) {
    auto *modelObj = dynamic_cast<Model3DObj *>(obj);
    if (! modelObj) { singleBuffer = false; break; }

    auto *geomObject = dynamic_cast<GeomObject *>(modelObj->object());
    if (! geomObject->refObject()) { singleBuffer = false; break; }

    auto *geomObject1 = dynamic_cast<GeomObject *>(geomObject->refObject());
    if (! geomObject1) { singleBuffer = false; break; }

    auto *buffer1 = geomObject1->buffer();

    if (! buffer)
      buffer = buffer1;
    else if (buffer1 != buffer) {
      singleBuffer = false; break;
    }
  }

  if (singleBuffer)
    std::cerr << "Single Buffer\n";
#endif

  //---

  for (auto *obj : objects_) {
    if (! obj || ! obj->isVisible())
      continue;

    obj->render();
  }

  //---

  if (canvas_->isShowBBox() || isSelected()) {
    calcBBox();

    createBBoxObj();
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
