#include <CQSandboxGeomObject3D.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxModel3DObj.h>

#include <CQGLBuffer.h>

namespace CQSandbox {

GeomObject3D::
GeomObject3D(CGeomScene3D *pscene, const std::string &name) :
 CGeomObject3D(pscene, name)
{
}

GeomObject3D::
GeomObject3D(const GeomObject3D &object) :
 CGeomObject3D(object)
{
}

GeomObject3D::
~GeomObject3D()
{
}

GeomObject3D *
GeomObject3D::
dup() const
{
  return new GeomObject3D(*this);
}

//---

CQGLBuffer *
GeomObject3D::
initBuffer(Canvas3D * /*canvas*/)
{
  auto *program = Model3DObj::shaderProgram();
  assert(program);

  if (! buffer_)
    buffer_ = program->createBuffer();

  buffer_->clearBuffers();

  faceDatas_.clear();

  return buffer_;
}

void
GeomObject3D::
addFaceData(const FaceData &faceData)
{
  faceDatas_.push_back(faceData);
}

}
