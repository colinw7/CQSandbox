#include <CQSandboxGeomObject.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxModel3DObj.h>

#include <CQGLBuffer.h>

namespace CQSandbox {

GeomObject::
GeomObject(CGeomScene3D *pscene, const std::string &name) :
 CGeomObject3D(pscene, name)
{
}

GeomObject::
GeomObject(const GeomObject &object) :
 CGeomObject3D(object)
{
}

GeomObject::
~GeomObject()
{
}

GeomObject *
GeomObject::
dup() const
{
  return new GeomObject(*this);
}

//---

CQGLBuffer *
GeomObject::
initBuffer(Canvas3D * /*canvas*/)
{
  auto *program = Model3DObj::shaderProgram();
//auto *program = canvas->sceneShaderProgram();

  assert(program);

  if (! buffer_)
    buffer_ = program->createBuffer();

  buffer_->clearBuffers();

  faceDatas_.clear();

  return buffer_;
}

void
GeomObject::
addFaceData(const FaceData &faceData)
{
  faceDatas_.push_back(faceData);
}

}
