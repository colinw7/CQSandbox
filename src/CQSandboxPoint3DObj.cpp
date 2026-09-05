#include <CQSandboxPoint3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQGLBuffer.h>
#include <CQTclUtil.h>

namespace CQSandbox {

ShaderProgram *Point3DObj::s_program   = nullptr;
Point3DObjMgr *Point3DObj::s_objectMgr = nullptr;

//---

void
Point3DObjMgr::
initRender(Canvas3D *canvas)
{
  Point3DObj::initShader(canvas);

  Point3DObj::initDraw(canvas);
}

void
Point3DObjMgr::
termRender(Canvas3D *canvas)
{
  Point3DObj::termDraw(canvas);
}

//---

Object3D *
Point3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->tcl();

  auto *obj = new Point3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

Point3DObj::
Point3DObj(Canvas3D *canvas) :
 Object3D(canvas, Type::POINT)
{
  if (! s_objectMgr) {
    s_objectMgr = new Point3DObjMgr;

    canvas->addObjectMgr(s_objectMgr);
  }

  s_objectMgr->addObject(this);
}

void
Point3DObj::
init()
{
  Object3D::init();

  //---

  initShader(canvas_);

  buffer_ = s_program->createBuffer();
}

void
Point3DObj::
initShader(Canvas3D *canvas)
{
  if (s_program)
    return;

  auto *app = canvas->app();

  s_program = new ShaderProgram(canvas);

  s_program->addVertexFile  (app->buildDir() + "/shaders/point.vs");
  s_program->addFragmentFile(app->buildDir() + "/shaders/point.fs");

  s_program->link();
}

bool
Point3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  return Object3D::getValue(name, args, value);
}

bool
Point3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas_->tcl();

  if      (name == "size") {
    if (! Util::stringToReal(value, size_))
      return false;
  }
  else if (name == "color") {
    if (! Util::stringToColor(tcl, value, color_))
      return false;
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
Point3DObj::
updateGL()
{
  if (! needsUpdate_)
    return;

  needsUpdate_ = false;

  //---

  calcBBox();

  //---

  buffer_->clearBuffers();

  buffer_->addPoint(position_);
  buffer_->addColor(color_);

  buffer_->load();
}

CBBox3D
Point3DObj::
calcBBox()
{
  if (! bboxValid_) {
    bbox_ = CBBox3D();

    //bbox_ += position_;

    bboxValid_ = true;
  }

  return bbox_;
}

void
Point3DObj::
render()
{
  updateGL();

  //---

  canvas_->bindBuffer(buffer_);

  //---

  glPointSize(size_);

  glDrawArrays(GL_POINTS, 0, 1);

  //---

  canvas_->bindBuffer(nullptr);
}

void
Point3DObj::
initDraw(Canvas3D *canvas)
{
  canvas->bindProgram(s_program);

  //---

  canvas->setProgramMatrices(s_program);
}

void
Point3DObj::
termDraw(Canvas3D *canvas)
{
  canvas->bindProgram(nullptr);
}

}
