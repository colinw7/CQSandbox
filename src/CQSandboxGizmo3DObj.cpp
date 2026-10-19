#include <CQSandboxGizmo3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxShape3DObj.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CInterval.h>

namespace CQSandbox {

Object3D *
Gizmo3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->tcl();

  auto *obj = new Gizmo3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

Gizmo3DObj::
Gizmo3DObj(Canvas3D *canvas) :
 Object3D(canvas, Type::GIZMO)
{
  auto createCylinder = [&](const CPoint3D &angles, const QColor &c) {
    Shape3DData::ConfigData configData;

    auto *cylinder = new Shape3DObj(canvas);

    cylinder->addCylinder(0.01, 1, configData);

    cylinder->setAngles(angles);

    cylinder->setDiffuseColor(c);

    cylinder->setLighted(false);

    return cylinder;
  };

  xCylinder_ = createCylinder(CPoint3D(0,        0, -M_PI/2.0), Qt::red  );
  yCylinder_ = createCylinder(CPoint3D(0,        0,         0), Qt::green);
  zCylinder_ = createCylinder(CPoint3D(M_PI/2.0, 0,         0), Qt::blue );

  auto createCone = [&](const CPoint3D &d, const CPoint3D &angles, const QColor &c) {
    Shape3DData::ConfigData configData;

    auto *cone = new Shape3DObj(canvas);

    cone->addCone(0.02, 0.05, configData);

    cone->setPosition(CPoint3D(d.x, d.y, d.z));

    cone->setAngles(angles);

    cone->setDiffuseColor(c);

    cone->setLighted(false);

    return cone;
  };

  xCone_ = createCone(CPoint3D(1, 0, 0), CPoint3D(0       , 0, -M_PI/2.0), Qt::red  );
  yCone_ = createCone(CPoint3D(0, 1, 0), CPoint3D(0       , 0,         0), Qt::green);
  zCone_ = createCone(CPoint3D(0, 0, 1), CPoint3D(M_PI/2.0, 0,         0), Qt::blue );

  auto createTorus = [&](const CPoint3D &d, const CPoint3D &angles, const QColor &c) {
    Shape3DData::ConfigData configData;

    configData.num_patches = 10;
    configData.num_xy      = 40;

    auto *torus = new Shape3DObj(canvas);

    torus->addTorus(1, 0.02, 1, 1, configData);

    torus->setPosition(CPoint3D(d.x, d.y, d.z));

    torus->setAngles(angles);

    torus->setDiffuseColor(c);

    torus->setLighted(false);

    return torus;
  };

  xTorus_ = createTorus(CPoint3D(0, 0, 0), CPoint3D(0       , 0,         0), Qt::red  );
  yTorus_ = createTorus(CPoint3D(0, 0, 0), CPoint3D(0       , -M_PI/2.0, 0), Qt::green);
  zTorus_ = createTorus(CPoint3D(0, 0, 0), CPoint3D(M_PI/2.0, 0,         0), Qt::blue );

  //---

  setEditType(EditType::MOVE);

  connect(canvas_, SIGNAL(editModeChanged()), this, SLOT(updateEditType()));
}

void
Gizmo3DObj::
init()
{
  xCylinder_->init();
  yCylinder_->init();
  zCylinder_->init();

  xCone_->init();
  yCone_->init();
  zCone_->init();

  xTorus_->init();
  yTorus_->init();
  zTorus_->init();

  Object3D::init();
}

void
Gizmo3DObj::
updateEditType()
{
  if (parentObj()) {
    if      (canvas_->editMode() == Canvas3D::EditMode::MOVE)
      setEditType(EditType::MOVE);
    else if (canvas_->editMode() == Canvas3D::EditMode::SCALE)
      setEditType(EditType::SCALE);
    else if (canvas_->editMode() == Canvas3D::EditMode::ROTATE)
      setEditType(EditType::ROTATE);
  }
}

void
Gizmo3DObj::
setEditType(const EditType &t)
{
  editType_ = t;

  xCylinder_->setVisible(editType_ == EditType::MOVE || editType_ == EditType::SCALE);
  yCylinder_->setVisible(editType_ == EditType::MOVE || editType_ == EditType::SCALE);
  zCylinder_->setVisible(editType_ == EditType::MOVE || editType_ == EditType::SCALE);

  xCone_->setVisible(editType_ == EditType::MOVE);
  yCone_->setVisible(editType_ == EditType::MOVE);
  zCone_->setVisible(editType_ == EditType::MOVE);

  xTorus_->setVisible(editType_ == EditType::ROTATE);
  yTorus_->setVisible(editType_ == EditType::ROTATE);
  zTorus_->setVisible(editType_ == EditType::ROTATE);

  canvas_->update();
}

void
Gizmo3DObj::
setParentObj(Object3D *o)
{
  parentObj_ = o;

  updateEditType();
}

bool
Gizmo3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  return Object3D::getValue(name, args, value);
}

bool
Gizmo3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if (name == "edit_type") {
    auto lstr = value.toLower();

    if      (lstr == "move")
      setEditType(EditType::MOVE);
    else if (lstr == "scale")
      setEditType(EditType::SCALE);
    else if (lstr == "rotate")
      setEditType(EditType::ROTATE);
    else
      return false;
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
Gizmo3DObj::
updateObjects()
{
  if (! isNeedsUpdate())
    return;

  needsUpdate_ = false;

  //---

  updateModelMatrix();
}

void
Gizmo3DObj::
updateModelMatrix()
{
  setModelMatrix();
}

void
Gizmo3DObj::
loadBuffer()
{
  xCylinder_->loadBuffer();
  yCylinder_->loadBuffer();
  zCylinder_->loadBuffer();

  xCone_->loadBuffer();
  yCone_->loadBuffer();
  zCone_->loadBuffer();

  xTorus_->loadBuffer();
  yTorus_->loadBuffer();
  zTorus_->loadBuffer();
}

CBBox3D
Gizmo3DObj::
calcBBox()
{
  xCylinder_->calcBBox();
  yCylinder_->calcBBox();
  zCylinder_->calcBBox();

  xCone_->calcBBox();
  yCone_->calcBBox();
  zCone_->calcBBox();

  xTorus_->calcBBox();
  yTorus_->calcBBox();
  zTorus_->calcBBox();

  rawBBox_ = xCylinder_->getRawBBox() + yCylinder_->getRawBBox() + zCylinder_->getRawBBox() +
             xCone_    ->getRawBBox() + yCone_    ->getRawBBox() + zCone_    ->getRawBBox() +
             xTorus_   ->getRawBBox() + yTorus_   ->getRawBBox() + zTorus_   ->getRawBBox();

  bbox_ = xCylinder_->calcBBox() + yCylinder_->calcBBox() + zCylinder_->calcBBox() +
          xCone_    ->calcBBox() + yCone_    ->calcBBox() + zCone_    ->calcBBox() +
          xTorus_   ->calcBBox() + yTorus_   ->calcBBox() + zTorus_   ->calcBBox();

  return bbox_;
}

void
Gizmo3DObj::
render()
{
  CPoint3D pos;
  CPoint3D angles;
  double   xs { 1 }, ys { 1 }, zs { 1 };

  if (parentObj()) {
    auto bbox    = parentObj_->bbox();
    auto rawBBox = parentObj_->getRawBBox();

    pos = bbox.getCenter();

    xs = 1.1*rawBBox.getXSize();
    ys = 1.1*rawBBox.getYSize();
    zs = 1.1*rawBBox.getZSize();

    angles = parentObj_->angles();

    xCylinder_->setAngles(angles + CPoint3D(0,        0, -M_PI/2.0));
    yCylinder_->setAngles(angles + CPoint3D(0,        0,         0));
    zCylinder_->setAngles(angles + CPoint3D(M_PI/2.0, 0,         0));

    xTorus_->setAngles(angles + CPoint3D(0       , 0,         0));
    yTorus_->setAngles(angles + CPoint3D(0       , -M_PI/2.0, 0));
    zTorus_->setAngles(angles + CPoint3D(M_PI/2.0, 0,         0));
  }

  Shape3DObj::initDraw(canvas_);

  auto drawPart = [&](Object3D *part) {
    if (! part->isVisible())
      return;

    part->setPosition(pos);

    part->setScales(CPoint3D(xs, ys, zs));

    part->render();
  };

  drawPart(xCylinder_);
  drawPart(yCylinder_);
  drawPart(zCylinder_);

  drawPart(xCone_);
  drawPart(yCone_);
  drawPart(zCone_);

  drawPart(xTorus_);
  drawPart(yTorus_);
  drawPart(zTorus_);

  Shape3DObj::termDraw(canvas_);
}

}
