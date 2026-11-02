#include <CQSandboxMaterial3D.h>
#include <CQSandboxTexture.h>

namespace CQSandbox {

Material3D::
Material3D(Canvas3D *canvas) :
 canvas_(canvas)
{
}

void
Material3D::
setDiffuseTexture(CGeomTexture *t)
{
  diffuseTexture_ = dynamic_cast<Texture *>(t);
}

void
Material3D::
setNormalTexture(CGeomTexture *t)
{
  normalTexture_ = dynamic_cast<Texture *>(t);
}

void
Material3D::
setSpecularTexture(CGeomTexture *t)
{
  specularTexture_ = dynamic_cast<Texture *>(t);
}

void
Material3D::
setEmissiveTexture(CGeomTexture *t)
{
  emissiveTexture_ = dynamic_cast<Texture *>(t);
}

}
