#include <CQSandboxGrid3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQGLBuffer.h>
#include <CQGLState.h>

namespace CQSandbox {

ShaderProgram *Grid3DObj::s_program = nullptr;

Object3D *
Grid3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->tcl();

  auto *obj = new Grid3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

Grid3DObj::
Grid3DObj(Canvas3D *canvas) :
 Object3D(canvas, Type::AXIS)
{
}

void
Grid3DObj::
init()
{
  Object3D::init();

  //---

  initShader();

  //---

  buffer_ = s_program->createBuffer();
}

bool
Grid3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  return Object3D::getValue(name, args, value);
}

bool
Grid3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  return Object3D::setValue(name, value, args);
}

void
Grid3DObj::
initShader()
{
  if (! s_program) {
    auto *app = canvas_->app();

    s_program = new ShaderProgram(this);

    s_program->addVertexFile  (app->buildDir() + "/shaders/grid.vs");
    s_program->addFragmentFile(app->buildDir() + "/shaders/grid.fs");

    s_program->link();
  }
}

void
Grid3DObj::
updateGL()
{
  if (! needsUpdate_)
    return;

  needsUpdate_ = false;

  //---

  buffer_->addPoint(-1, -1, 0);
  buffer_->addPoint( 1, -1, 0);
  buffer_->addPoint( 1,  1, 0);

  buffer_->addPoint(-1, -1, 0);
  buffer_->addPoint( 1,  1, 0);
  buffer_->addPoint(-1,  1, 0);

  //---

  buffer_->load();
}

void
Grid3DObj::
render()
{
  updateGL();

  //---

  canvas_->bindProgram(s_program);

  canvas_->setProgramMatrices(s_program);

  auto bbox = canvas_->bbox();
  auto size = bbox.getMaxSize();

  s_program->setUniformValue("gridSize", float(3*size));

  //---

  auto oldBlend = CQGLStateInst->setBlend(true);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  bool oldDepthTest = CQGLStateInst->setDepthTest(false);
  bool oldCullFace  = CQGLStateInst->setCullFace(false);

  //---

  canvas_->bindBuffer(buffer_);

  glDrawArrays(GL_TRIANGLES, 0, 6);

  //---

  CQGLStateInst->setBlend(oldBlend);

  CQGLStateInst->setDepthTest(oldDepthTest);
  CQGLStateInst->setCullFace(oldCullFace);

  //---

  canvas_->bindBuffer(nullptr);
  canvas_->bindProgram(nullptr);
}

}
