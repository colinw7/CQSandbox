#include <CQGLTexture.h>
#include <CMathGen.h>

#include <QFileInfo>
#include <QImageReader>
#include <iostream>

#if 0
#include <glad/glad.h>
#endif
#include <GL/glut.h>

namespace {

bool checkError() {
  // check texture generated
  GLenum err = glGetError();

  if (err != GL_NO_ERROR) {
    std::cerr << "OpenGL Error: " << gluErrorString(err) << "\n";
    return false;
  }

  return true;
}

}

//---

CQGLTexture::
CQGLTexture()
{
}

CQGLTexture::
CQGLTexture(const QImage &image)
{
  setImage(image);
}

CQGLTexture::
~CQGLTexture()
{
  if (valid_ && glIsTexture(id_))
    glDeleteTextures(1, &id_);
}

bool
CQGLTexture::
load(const QString &fileName, bool flip)
{
  QFileInfo fi(fileName);

  if (! fi.exists(fileName)) {
    std::cerr << "Error: Invalid texture file '" << fileName.toStdString() << "'\n";
    return false;
  }

  QImageReader imageReader(fileName);

  QImage image;

  if (! imageReader.read(&image))
    return false;

  if (image.isNull()) {
    std::cerr << "Error: Failed to read image from '" << fileName.toStdString() << "'\n";
    return false;
  }

  return load(image, flip);
}

bool
CQGLTexture::
load(const QImage &image, bool flip)
{
  return init(image, flip);
}

void
CQGLTexture::
setImage(const QImage &image)
{
  init(image, /*flip*/false);
}

bool
CQGLTexture::
init(const QImage &image, bool flip)
{
  if (image.isNull()) {
    std::cerr << "Invalid image data\n";
    return false;
  }

  if (useAlpha())
    image_ = image.convertToFormat(QImage::Format_RGBA8888);
  else
    image_ = image.convertToFormat(QImage::Format_RGB888);

//if (flip)
//  image_ = image_.flippedH();

  auto w = image_.width ();
  auto h = image_.height();

  imageData_ = new unsigned char [4*w*h];

  int i = 0;

  for (int y = 0; y < h; ++y) {
    int y1 = (flip ? h - 1 - y : y);

    for (int x = 0; x < w; ++x) {
      auto rgba = image_.pixel(x, y1);

      imageData_[i++] = qBlue (rgba);
      imageData_[i++] = qGreen(rgba);
      imageData_[i++] = qRed  (rgba);
      imageData_[i++] = qAlpha(rgba);
    }
  }

  //------

  //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // allocate texture id
  glGenTextures(1, &id_);
  if (! checkError()) return false;

  valid_ = true;

  // set texture type
  glBindTexture(GL_TEXTURE_2D, id_);
  if (! checkError()) return false;

  if (wrapType() == WrapType::CLAMP) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }
  if (! checkError()) return false;

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  if (! checkError()) return false;

  // select modulate to mix texture with color for shading
  //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  //if (! checkError()) return false;

  // build our texture mipmaps
  GLint internalFormat = (useAlpha() ? GL_RGBA : GL_RGB);

  if (CMathGen::isPowerOf(2, w) && CMathGen::isPowerOf(2, h)) {
#if 1
    // Hardware mipmap generation
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
    if (! checkError()) return false;

    glHint(GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST);
    if (! checkError()) return false;
#endif

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0,
                 GL_BGRA, GL_UNSIGNED_BYTE, &imageData_[0]);
    if (! checkError()) return false;

#if 0
    glGenerateMipmap(GL_TEXTURE_2D);
    if (! checkError()) return false;
#endif
  }
  else {
    // No hardware mipmap generation support, fall back to the
    // good old gluBuild2DMipmaps function
    gluBuild2DMipmaps(GL_TEXTURE_2D, internalFormat, w, h,
                      GL_BGRA, GL_UNSIGNED_BYTE, &imageData_[0]);
    if (! checkError()) return false;
  }

  return true;
}

void
CQGLTexture::
bind() const
{
  glBindTexture(GL_TEXTURE_2D, id_);
}

void
CQGLTexture::
unbind() const
{
  glBindTexture(GL_TEXTURE_2D, 0);
}

void
CQGLTexture::
enable(bool b)
{
  if (b)
    glEnable(GL_TEXTURE_2D);
  else
    glDisable(GL_TEXTURE_2D);
}

void
CQGLTexture::
draw()
{
  draw(0.0, 0.0, 1.0, 1.0);
}

void
CQGLTexture::
draw(double x1, double y1, double x2, double y2)
{
  draw(x1, y1, 0, x2, y2, 0, 0, 0, 1, 1);
}

void
CQGLTexture::
draw(double x1, double y1, double z1, double x2, double y2, double z2,
     double tx1, double ty1, double tx2, double ty2)
{
  //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  enable(true);

  bind();

  // select modulate to mix texture with color for shading
  //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glBegin(GL_QUADS);

  if      (fabs(z2 - z1) < 1E-3) {
    glTexCoord2d(tx1, ty1); glVertex3d(x1, y1, z1);
    glTexCoord2d(tx1, ty2); glVertex3d(x1, y2, z1);
    glTexCoord2d(tx2, ty2); glVertex3d(x2, y2, z1);
    glTexCoord2d(tx2, ty1); glVertex3d(x2, y1, z1);
  }
  else if (fabs(y2 - y1) < 1E-3) {
    glTexCoord2d(tx1, ty1); glVertex3d(x1, y1, z1);
    glTexCoord2d(tx1, ty2); glVertex3d(x1, y1, z2);
    glTexCoord2d(tx2, ty2); glVertex3d(x2, y1, z2);
    glTexCoord2d(tx2, ty1); glVertex3d(x2, y1, z1);
  }
  else {
    glTexCoord2d(tx1, ty1); glVertex3d(x1, y1, z1);
    glTexCoord2d(tx1, ty2); glVertex3d(x1, y2, z1);
    glTexCoord2d(tx2, ty2); glVertex3d(x1, y2, z2);
    glTexCoord2d(tx2, ty1); glVertex3d(x1, y1, z2);
  }

  glEnd();

  enable(false);
}
