#include <CQSandboxLight3D.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxShaderProgram.h>

#include <CQGLBuffer.h>
#include <CQGLUtil.h>

namespace CQSandbox {

ShaderProgram* Light3D::s_program;

Light3D::
Light3D(Canvas3D *canvas, const Type &type) :
 canvas_(canvas), type_(type)
{
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
    s_program = new ShaderProgram;

    s_program->addVertexFile  (canvas_->buildDir() + "/shaders/light.vs");
    s_program->addFragmentFile(canvas_->buildDir() + "/shaders/light.fs");

    s_program->link();
  }
}

void
Light3D::
render()
{
  initBuffer();

  // setup light shader
  buffer_->bind();

  s_program->bind();

  s_program->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  s_program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  auto lightMatrix = CGLMatrix3D::translation(position());
  lightMatrix.scaled(0.01f, 0.01f, 0.01f);
  s_program->setUniformValue("model", CQGLUtil::toQMatrix(lightMatrix));

  s_program->setUniformValue("color", CQGLUtil::toVector(color()));

  // draw light
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  //buffer_->drawTriangles();

  s_program->release();

  buffer_->unbind();
}

}
