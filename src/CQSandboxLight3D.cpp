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
 CameraIFace(canvas->app()), CGeomLight3D(canvas->scene()), canvas_(canvas)
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
setName(const std::string &name)
{
  CGeomLight3D::setName(name);
}

const CVector3D &
Light3D::
position() const
{
  position_ = CVector3D(CGeomLight3D::getPosition());

  return position_;
}

void
Light3D::
setPosition(const CVector3D &p)
{
  CGeomLight3D::setPosition(p.point());
}

const CVector3D &
Light3D::
getDirection() const
{
  return CGeomLight3D::getDirection();
}

void
Light3D::
setDirection(const CVector3D &dir)
{
  CGeomLight3D::setDirection(dir);

  front_ = getDirection();
}

void
Light3D::
reset(const CBBox3D &)
{
}

// coordinate system vectors
CVector3D
Light3D::
front() const
{
  return front_;
}

CVector3D
Light3D::
up() const
{
  return up_;
}

CVector3D
Light3D::
right() const
{
  return front().crossProduct(up());
}

CMatrix3DH
Light3D::
perspectiveMatrix() const
{
  return CMatrix3DH::perspective(fov(), aspect(), near(), far());
}

CMatrix3DH
Light3D::
orthoMatrix() const
{
  const auto &bbox = canvas_->bbox();

  return CMatrix3DH::ortho(bbox.getXMin(), bbox.getXMax(), bbox.getYMin(), bbox.getYMax(),
                           near(), far());
}

CMatrix3DH
Light3D::
viewMatrix() const
{
  CMatrix3DH m;

  m.setLookAt(position().point(), front(), up(), right());

  return m;
}

double
Light3D::
distance() const
{
  return 1.0;
}

void
Light3D::
setDistance(double)
{
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
  canvas_->bindBuffer(buffer_);

  canvas_->bindProgram(s_program);

  canvas_->setProgramMatrices(s_program);

  auto lightMatrix =
    CMatrix3D::translation(getPosition().getX(), getPosition().getY(), getPosition().getZ());
  lightMatrix.scaled(0.01, 0.01, 0.01);
  s_program->setUniformValue("model", CQGLUtil::toQMatrix(lightMatrix));

  s_program->setUniformValue("color", CQGLUtil::toVector(getDiffuse()));

  // draw light
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  canvas_->bindProgram(nullptr);

  canvas_->bindBuffer(nullptr);
}

void
Light3D::
notifyChanged()
{
  Q_EMIT changedSignal();
}

}
