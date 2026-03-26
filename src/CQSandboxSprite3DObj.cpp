#include <CQSandboxSprite3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQGLBuffer.h>
#include <CQGLTexture.h>
#include <CQGLUtil.h>

namespace CQSandbox {

ShaderProgram *Sprite3DObj::s_program = nullptr;

bool
Sprite3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Sprite3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Sprite3DObj::
Sprite3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

void
Sprite3DObj::
init()
{
  Object3D::init();

  //---

  if (! s_program) {
#if 0
    static const char *vertexShaderSource =
      "#version 330 core\n"
      "layout (location = 0) in vec3 aPos;\n"
      "layout (location = 1) in vec2 aTexCoord;\n"
      "out vec2 TexCoord;\n"
      "uniform mat4 model;\n"
      "uniform mat4 view;\n"
      "void main() {\n"
      "  TexCoord = aTexCoord;\n"
      "  //gl_Position = vec4(aPos, 1.0);\n"
      "  gl_Position = view * model * vec4(aPos, 1.0);\n"
      "}\n";
    static const char *fragmentShaderSource =
      "#version 330 core\n"
      "out vec4 FragColor;\n"
      "in vec2 TexCoord;\n"
      "uniform sampler2D textureId;\n"
      "void main() {\n"
      "  vec4 texColor = texture(textureId, TexCoord);\n"
      "  if (texColor.a < 0.1) {\n"
      "    discard;\n"
      "  }\n"
      " FragColor = texColor;\n"
      "}\n";
#endif

    s_program = new ShaderProgram(this);

#if 0
    s_program->addVertexCode  (vertexShaderSource);
    s_program->addFragmentCode(fragmentShaderSource);
#else
    s_program->addVertexFile  (canvas_->buildDir() + "/shaders/sprite.vs");
    s_program->addFragmentFile(canvas_->buildDir() + "/shaders/sprite.fs");
#endif

    s_program->link();
  }

  //---

  updateBuffer();
}

void
Sprite3DObj::
updateBuffer()
{
  delete buffer_;

  buffer_ = s_program->createBuffer();

  auto z = position().z;

  buffer_->addPoint(-1.0f,  1.0f, float(z)); buffer_->addTexturePoint(0.0f, 0.0f);
  buffer_->addPoint(-1.0f, -1.0f, float(z)); buffer_->addTexturePoint(0.0f, 1.0f);
  buffer_->addPoint( 1.0f,  1.0f, float(z)); buffer_->addTexturePoint(1.0f, 0.0f);
  buffer_->addPoint( 1.0f, -1.0f, float(z)); buffer_->addTexturePoint(1.0f, 1.0f);

  buffer_->load();
}

void
Sprite3DObj::
setTexture(CQGLTexture *texture)
{
  textures_.clear();

  textures_.push_back(texture);
}

QVariant
Sprite3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

bool
Sprite3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas_->app();
  auto *tcl = app->tcl();

  if      (name == "add_image") {
    auto *texture = new CQGLTexture;

    if (texture->load(value, /*flip*/false))
      textures_.push_back(texture);
    else
      delete texture;
  }
  else if (name == "image_start") {
    textureStart_ = Util::stringToInt(value);
  }
  else if (name == "image_end") {
    textureEnd_ = Util::stringToInt(value);
  }
  else if (name == "velocity") {
    auto v = Util::stringToPoint2D(tcl, value);

    xv_ = v.x;
    yv_ = v.y;
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
Sprite3DObj::
tick()
{
  updateObjects();

  Object3D::tick();

  position_ = CPoint3D(position_.x + xv_, position_.y + yv_, position_.z);

  if (ticks_ % 100) {
    int textureEnd = textureEnd_;

    if (textureEnd < 0)
      textureEnd = int(textures_.size()) - 1;

    int numTextures = std::max(textureEnd - textureStart_ + 1, 0);

    ++textureNum_;

    if (textureNum_ >= numTextures)
      textureNum_ = 0;
  }
}

void
Sprite3DObj::
updateObjects()
{
  if (! needsUpdate_)
    return;

  needsUpdate_ = false;

  //---

  updateModelMatrix();
}

void
Sprite3DObj::
updateModelMatrix()
{
  xAngle_ = 2.0;
  yAngle_ = 0.0;
  zscale_ = 1.0;

  setModelMatrix();
}

void
Sprite3DObj::
render()
{
  buffer_->bind();

  //---

  s_program->bind();

  s_program->setUniformValue("textureId", 0);

  s_program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  updateModelMatrix();

  s_program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix()));

  glActiveTexture(GL_TEXTURE0);

  int textureNum = textureNum_ + textureStart_;

  if (textureNum >= 0 && textureNum < int(textures_.size()))
    textures_[textureNum]->bind();

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  s_program->release();

  buffer_->unbind();
}

}
