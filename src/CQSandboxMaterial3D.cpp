#include <CQSandboxMaterial3D.h>

#include <CQGLTexture.h>

namespace CQSandbox {

Material3D::
Material3D()
{
}

void
Material3D::
setDiffuseTexture(CQGLTexture *t)
{
  delete diffuseTexture_;

  diffuseTexture_ = t;
}

void
Material3D::
setNormalTexture(CQGLTexture *t)
{
  delete normalTexture_;

  normalTexture_ = t;
}

void
Material3D::
setSpecularTexture(CQGLTexture *t)
{
  delete specularTexture_;

  specularTexture_ = t;
}

void
Material3D::
setEmissionTexture(CQGLTexture *t)
{
  delete emissionTexture_;

  emissionTexture_ = t;
}

}
