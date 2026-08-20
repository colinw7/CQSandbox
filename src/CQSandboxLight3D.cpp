#include <CQSandboxLight3D.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxShaderProgram.h>
#include <CQSandboxApp.h>

#include <CQGLBuffer.h>
#include <CQGLUtil.h>

namespace CQSandbox {

ShaderProgram* Light3D::s_program;

Light3D::
Light3D(Canvas3D *canvas, const Type &type) :
 CGeomLight3D(canvas->scene()), canvas_(canvas)
{
  setType(type);
}

Light3D::
~Light3D()
{
  delete buffer_;
}

void
Light3D::
initBuffer()
{
  initShader();

  // set up vertex data (and buffer(s)) and configure vertex attributes
  if (! buffer_) {
    buffer_ = s_program->createBuffer();

    auto addPoint = [&](double x, double y, double z) {
      buffer_->addPoint(x, y, z);
    };

    addPoint(-0.5f, -0.5f, -0.5f); addPoint( 0.5f, -0.5f, -0.5f); addPoint( 0.5f,  0.5f, -0.5f);
    addPoint( 0.5f,  0.5f, -0.5f); addPoint(-0.5f,  0.5f, -0.5f); addPoint(-0.5f, -0.5f, -0.5f);
    addPoint(-0.5f, -0.5f,  0.5f); addPoint( 0.5f, -0.5f,  0.5f); addPoint( 0.5f,  0.5f,  0.5f);
    addPoint( 0.5f,  0.5f,  0.5f); addPoint(-0.5f,  0.5f,  0.5f); addPoint(-0.5f, -0.5f,  0.5f);
    addPoint(-0.5f,  0.5f,  0.5f); addPoint(-0.5f,  0.5f, -0.5f); addPoint(-0.5f, -0.5f, -0.5f);
    addPoint(-0.5f, -0.5f, -0.5f); addPoint(-0.5f, -0.5f,  0.5f); addPoint(-0.5f,  0.5f,  0.5f);
    addPoint( 0.5f,  0.5f,  0.5f); addPoint( 0.5f,  0.5f, -0.5f); addPoint( 0.5f, -0.5f, -0.5f);
    addPoint( 0.5f, -0.5f, -0.5f); addPoint( 0.5f, -0.5f,  0.5f); addPoint( 0.5f,  0.5f,  0.5f);
    addPoint(-0.5f, -0.5f, -0.5f); addPoint( 0.5f, -0.5f, -0.5f); addPoint( 0.5f, -0.5f,  0.5f);
    addPoint( 0.5f, -0.5f,  0.5f); addPoint(-0.5f, -0.5f,  0.5f); addPoint(-0.5f, -0.5f, -0.5f);
    addPoint(-0.5f,  0.5f, -0.5f); addPoint( 0.5f,  0.5f, -0.5f); addPoint( 0.5f,  0.5f,  0.5f);
    addPoint( 0.5f,  0.5f,  0.5f); addPoint(-0.5f,  0.5f,  0.5f); addPoint(-0.5f,  0.5f, -0.5f);

    buffer_->load();
  }
}

void
Light3D::
initShader()
{
  if (! s_program) {
    auto *app = canvas_->app();

    s_program = new ShaderProgram;

    s_program->addVertexFile  (app->buildDir() + "/shaders/light.vs");
    s_program->addFragmentFile(app->buildDir() + "/shaders/light.fs");

    s_program->link();
  }
}

void
Light3D::
render()
{
  initBuffer();

  // setup light shader
  //buffer_->bind();
  canvas_->bindBuffer(buffer_);

  //s_program->bind();
  canvas_->bindProgram(s_program);

  s_program->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  s_program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  auto lightMatrix =
    CMatrix3D::translation(getPosition().getX(), getPosition().getY(), getPosition().getZ());
  lightMatrix.scaled(0.01, 0.01, 0.01);
  s_program->setUniformValue("model", CQGLUtil::toQMatrix(lightMatrix));

  s_program->setUniformValue("color", CQGLUtil::toVector(getDiffuse()));

  // draw light
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  //buffer_->drawTriangles();

  //s_program->release();

  //buffer_->unbind();
}

void
Light3D::
notifyChanged()
{
  Q_EMIT changedSignal();
}

}
