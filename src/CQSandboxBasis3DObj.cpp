#include <CQSandboxBasis3DObj.h>
#include <CQSandboxShape3DObj.h>
#include <CQSandboxModel3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>

#include <CGeomObject3D.h>
#include <CQTclUtil.h>
#include <CQGLBuffer.h>
#include <CQGLUtil.h>
#include <CQGLState.h>

namespace CQSandbox {

Object3D *
Basis3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->tcl();

  auto *obj = new Basis3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

Basis3DObj::
Basis3DObj(Canvas3D *canvas) :
 Object3D(canvas, Type::BASIS)
{
  auto createCylinder = [&](const CPoint3D &angles, const QColor &c) {
    Shape3DData::ConfigData configData;

    auto *cylinder = new Shape3DObj(canvas);

    cylinder->addCylinder(0.01, 1, configData);

    cylinder->setAngles(angles);

    cylinder->setDiffuseColor(c);

    cylinder->setPseudo(true);

    cylinder->setLighted(false);

    return cylinder;
  };

  xCylinder_ = createCylinder(CPoint3D(0,        0, -M_PI/2.0), Qt::red  );
  yCylinder_ = createCylinder(CPoint3D(0,        0,         0), Qt::green);
  zCylinder_ = createCylinder(CPoint3D(M_PI/2.0, 0,         0), Qt::blue );
}

void
Basis3DObj::
init()
{
  xCylinder_->init();
  yCylinder_->init();
  zCylinder_->init();

  Object3D::init();
}

void
Basis3DObj::
setParentObj(Object3D *o)
{
  parentObj_ = o;
}

bool
Basis3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  return Object3D::getValue(name, args, value);
}

bool
Basis3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  return Object3D::setValue(name, value, args);
}

void
Basis3DObj::
loadBuffer()
{
  xCylinder_->loadBuffer();
  yCylinder_->loadBuffer();
  zCylinder_->loadBuffer();
}

CBBox3D
Basis3DObj::
calcBBox()
{
  return bbox_;
}

void
Basis3DObj::
render()
{
  if (! parentObj_)
    return;

  auto *model3DObj = dynamic_cast<Model3DObj *>(parentObj_);

  CVector3D u1, v1, w1;

  if (model3DObj)
    model3DObj->object()->getBasis(u1, v1, w1);
  else {
    u1 = CVector3D(1, 0, 0);
    v1 = CVector3D(0, 1, 0);
    w1 = CVector3D(0, 0, 1);
  }

  const auto &modelMatrix = parentObj_->modelMatrix();

  auto u = (modelMatrix*u1).normalized();
  auto v = (modelMatrix*v1).normalized();
  auto w = (modelMatrix*w1).normalized();

  auto qu = CQuaternion::rotationArc(u1, u);
  auto qv = CQuaternion::rotationArc(v1, v);
  auto qw = CQuaternion::rotationArc(w1, w);

  double upitch, uyaw, uroll;
  qu.toEuler(upitch, uyaw, uroll);

  double vpitch, vyaw, vroll;
  qv.toEuler(vpitch, vyaw, vroll);

  double wpitch, wyaw, wroll;
  qw.toEuler(wpitch, wyaw, wroll);

  xCylinder_->setAngles(CPoint3D(upitch, uyaw, uroll));
  yCylinder_->setAngles(CPoint3D(vpitch, vyaw, vroll));
  zCylinder_->setAngles(CPoint3D(wpitch, wyaw, wroll));

  //---

  Shape3DObj::initDraw(canvas_);

  xCylinder_->render();
  yCylinder_->render();
  zCylinder_->render();

  Shape3DObj::termDraw(canvas_);
}

}
