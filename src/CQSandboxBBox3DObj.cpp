#include <CQSandboxBBox3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>

#include <CQTclUtil.h>
#include <CQGLBuffer.h>
#include <CQGLUtil.h>
#include <CQGLState.h>

namespace CQSandbox {

ShaderProgram *BBox3DObj::s_program = nullptr;

Object3D *
BBox3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->tcl();

  auto *obj = new BBox3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

BBox3DObj::
BBox3DObj(Canvas3D *canvas) :
 Object3D(canvas, Type::BBOX)
{
}

void
BBox3DObj::
init()
{
  Object3D::init();

  //---

  initShader();

  //---

  buffer_ = s_program->createBuffer();
}

void
BBox3DObj::
initShader()
{
  if (! s_program) {
    auto *app = canvas_->app();

    s_program = new ShaderProgram(this);

    s_program->addVertexFile  (app->buildDir() + "/shaders/bbox.vs");
    s_program->addFragmentFile(app->buildDir() + "/shaders/bbox.fs");

    s_program->link();
  }
}

void
BBox3DObj::
updateGL()
{
  if (! needsUpdate_)
    return;

  needsUpdate_ = false;

  //---

  static Points points = {
    CGLVector3D(-0.5f, -0.5f, -0.5f),
    CGLVector3D( 0.5f, -0.5f, -0.5f),
    CGLVector3D( 0.5f,  0.5f, -0.5f),

    CGLVector3D( 0.5f,  0.5f, -0.5f),
    CGLVector3D(-0.5f,  0.5f, -0.5f),
    CGLVector3D(-0.5f, -0.5f, -0.5f),

    CGLVector3D(-0.5f, -0.5f,  0.5f),
    CGLVector3D( 0.5f, -0.5f,  0.5f),
    CGLVector3D( 0.5f,  0.5f,  0.5f),

    CGLVector3D( 0.5f,  0.5f,  0.5f),
    CGLVector3D(-0.5f,  0.5f,  0.5f),
    CGLVector3D(-0.5f, -0.5f,  0.5f),

    CGLVector3D(-0.5f,  0.5f,  0.5f),
    CGLVector3D(-0.5f,  0.5f, -0.5f),
    CGLVector3D(-0.5f, -0.5f, -0.5f),

    CGLVector3D(-0.5f, -0.5f, -0.5f),
    CGLVector3D(-0.5f, -0.5f,  0.5f),
    CGLVector3D(-0.5f,  0.5f,  0.5f),

    CGLVector3D( 0.5f,  0.5f,  0.5f),
    CGLVector3D( 0.5f,  0.5f, -0.5f),
    CGLVector3D( 0.5f, -0.5f, -0.5f),

    CGLVector3D( 0.5f, -0.5f, -0.5f),
    CGLVector3D( 0.5f, -0.5f,  0.5f),
    CGLVector3D( 0.5f,  0.5f,  0.5f),

    CGLVector3D(-0.5f, -0.5f, -0.5f),
    CGLVector3D( 0.5f, -0.5f, -0.5f),
    CGLVector3D( 0.5f, -0.5f,  0.5f),

    CGLVector3D( 0.5f, -0.5f,  0.5f),
    CGLVector3D(-0.5f, -0.5f,  0.5f),
    CGLVector3D(-0.5f, -0.5f, -0.5f),

    CGLVector3D(-0.5f,  0.5f, -0.5f),
    CGLVector3D( 0.5f,  0.5f, -0.5f),
    CGLVector3D( 0.5f,  0.5f,  0.5f),

    CGLVector3D( 0.5f,  0.5f,  0.5f),
    CGLVector3D(-0.5f,  0.5f,  0.5f),
    CGLVector3D(-0.5f,  0.5f, -0.5f)
  };

  //---

  points_ = points;

  auto np = points_.size();

  buffer_->clearBuffers();

  for (uint i = 0; i < np; ++i)
    buffer_->addPoint(points_[i]);

  buffer_->load();
}

void
BBox3DObj::
render()
{
  updateGL();

  //---

  canvas_->bindProgram(s_program);

  canvas_->setProgramMatrices(s_program);

  setModelMatrix();
  s_program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix()));

  //---

  canvas_->bindBuffer(buffer_);

  CQGLStateInst->setPolygonMode(GL_LINE);

  bool oldCullFace = CQGLStateInst->setCullFace(false);

  //---

  int np = points_.size();

  glDrawArrays(GL_TRIANGLES, 0, np);

  //---

  CQGLStateInst->setCullFace(oldCullFace);

  //---

  canvas_->bindBuffer(nullptr);
  canvas_->bindProgram(nullptr);
}

}
