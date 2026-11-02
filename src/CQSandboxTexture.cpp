#include <CQSandboxTexture.h>
#include <CQSandboxCanvas3D.h>

namespace CQSandbox {

Texture::
Texture(Canvas3D *canvas) :
 canvas_(canvas)
{
}

CQGLTexture *
Texture::
glTexture(Canvas3D *canvas)
{
  auto &canvasTexture = canvasTextureData_[canvas->ind()];

  if (flipped_)
    return canvasTexture.glTextureFlipped;
  else
    return canvasTexture.glTexture;
}

void
Texture::
setWrapped(bool b)
{
  wrapped_ = b;

  for (auto pw : canvasTextureData_) {
    auto &textureData = pw.second;

    if (textureData.glTexture)
      textureData.glTexture->setWrapType(wrapped_ ?
       CQGLTexture::WrapType::REPEAT : CQGLTexture::WrapType::CLAMP);

    if (textureData.glTextureFlipped)
      textureData.glTextureFlipped->setWrapType(wrapped_ ?
       CQGLTexture::WrapType::REPEAT : CQGLTexture::WrapType::CLAMP);
  }
}

void
Texture::
setGlTextures(Canvas3D *canvas, CQGLTexture *t1, CQGLTexture *t2)
{
  auto &textureData = canvasTextureData_[canvas->ind()];

  textureData.glTexture        = t1;
  textureData.glTextureFlipped = t2;

  setWrapped(wrapped_);
}

}
