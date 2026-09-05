#include <CQSandboxPoint3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQGLBuffer.h>
#include <CQTclUtil.h>

namespace CQSandbox {

ShaderProgram *Point3DObj::s_program = nullptr;

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
 Shape3DObj(canvas)
{
}

void
Point3DObj::
init()
{
  Object3D::init();

  //---

  initShader();

  //---

  buffer_ = s_program->createBuffer();
}

void
Point3DObj::
initShader()
{
  if (! s_program) {
    auto *app = canvas_->app();

    s_program = new ShaderProgram(this);

    s_program->addVertexFile  (app->buildDir() + "/shaders/point.vs");
    s_program->addFragmentFile(app->buildDir() + "/shaders/point.fs");

    s_program->link();
  }
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

  buffer_->clearBuffers();

  buffer_->addPoint(position_);
  buffer_->addColor(color_);

  buffer_->load();
}

void
Point3DObj::
render()
{
  updateGL();

  //---

  //s_program->bind();
  canvas_->bindProgram(s_program);

  canvas_->setProgramMatrices(s_program);

  //---

  //buffer_->bind();
  canvas_->bindBuffer(buffer_);

  //---

  glPointSize(size_);

  glDrawArrays(GL_POINTS, 0, 1);
}

}
