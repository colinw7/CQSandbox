#include <CQSandboxPlane3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>
#include <CQGLTexture.h>
#include <CQGLUtil.h>

namespace CQSandbox {

ShaderProgram *Plane3DObj::s_program = nullptr;

bool
Plane3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Plane3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Plane3DObj::
Plane3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

void
Plane3DObj::
init()
{
  Object3D::init();

  //---

  initShader();

  //---

  canvas_->glGenVertexArrays(1, &vertexArrayId_);

  canvas_->glGenBuffers(1, &pointsBufferId_);
  canvas_->glGenBuffers(1, &colorsBufferId_);
  canvas_->glGenBuffers(1, &texCoordBufferId_);

  //---

  static Points points = {
    CGLVector3D(-0.5f, -0.5f, 0.0f),
    CGLVector3D( 0.5f, -0.5f, 0.0f),
    CGLVector3D( 0.5f,  0.5f, 0.0f),

    CGLVector3D( 0.5f,  0.5f, 0.0f),
    CGLVector3D(-0.5f,  0.5f, 0.0f),
    CGLVector3D(-0.5f, -0.5f, 0.0f),
  };

  static TexCoords texCoords = {
    CGLVector2D(0.0f, 0.0f),
    CGLVector2D(1.0f, 0.0f),
    CGLVector2D(1.0f, 1.0f),

    CGLVector2D(1.0f, 1.0f),
    CGLVector2D(0.0f, 1.0f),
    CGLVector2D(0.0f, 0.0f),
  };

  points_    = points;
  texCoords_ = texCoords;
}

bool
Plane3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas_->app();
  auto *tcl = app->tcl();

  if      (name == "color")
    setColor(Util::stringToQColor(tcl, value));
  else if (name == "texture")
    setTextureFile(value);
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
Plane3DObj::
setColor(const QColor &c)
{
  color_ = c;

  setNeedsUpdate();
}

void
Plane3DObj::
setTextureFile(const QString &filename)
{
  textureFile_ = filename;

  if (textureFile_ != "") {
    texture_ = new CQGLTexture;

    if (! texture_->load(textureFile_, /*flip*/true)) {
      delete texture_;
      texture_ = nullptr;
    }
  }
  else {
    delete texture_;
    texture_ = nullptr;
  }

  setNeedsUpdate();
}

void
Plane3DObj::
initShader()
{
  if (! s_program) {
#if 0
    static const char *vertexShaderSource =
      "#version 330 core\n"
      "layout (location = 0) in vec3 aPos;\n"
      "layout (location = 1) in vec4 aColor;\n"
      "layout (location = 2) in vec4 aTexCoord;\n"
      "uniform highp mat4 projection;\n"
      "uniform highp mat4 view;\n"
      "uniform highp mat4 model;\n"
      "out vec4 Color;\n"
      "out vec2 TexCoord;\n"
      "void main() {\n"
      "  Color    = aColor;\n"
      "  TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
      "  gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
      "}";
    static const char *fragmentShaderSource =
      "#version 330 core\n"
      "in vec4 Color;\n"
      "in vec2 TexCoord;\n"
      "out vec4 FragColor;\n"
      "uniform sampler2D textureId;\n"
      "uniform bool useTexture;\n"
      "void main() {\n"
      "  if (useTexture) {\n"
      "    FragColor = texture(textureId, TexCoord);\n"
      "  } else {\n"
      "    FragColor = Color;\n"
      "  }\n"
      "}";
#endif

    s_program = new ShaderProgram(this);

#if 0
    s_program->addVertexCode  (vertexShaderSource);
    s_program->addFragmentCode(fragmentShaderSource);
#else
    s_program->addVertexFile  (canvas_->buildDir() + "/shaders/plane.vs");
    s_program->addFragmentFile(canvas_->buildDir() + "/shaders/plane.fs");
#endif

    s_program->link();
  }
}

void
Plane3DObj::
updateGL()
{
  if (! needsUpdate_)
    return;

  needsUpdate_ = false;

  //---

  // bind the Vertex Array Object
  canvas_->glBindVertexArray(vertexArrayId_);

  //---

  auto np = points_.size();

  // store point data in array buffer (vec3, location 0)
  uint aPos = 0;
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, pointsBufferId_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(CGLVector3D), &points_[0], GL_STATIC_DRAW);

  // set points attrib data and format (for current buffer)
  canvas_->glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, sizeof(CGLVector3D), nullptr);
  canvas_->glEnableVertexAttribArray(aPos);

  //---

  // set colors attrib data and format (for current buffer) (vec4, location 2)
  std::vector<CGLColor> colors1;

  if (colors_.size() != np) {
    auto c  = this->color();
    auto c1 = Util::qcolorToColor(c);

    while (colors1.size() < np)
      colors1.push_back(c1);
  }
  else
    colors1 = colors_;

  uint aColor = 1;
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, colorsBufferId_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(CGLColor), &colors1[0], GL_STATIC_DRAW);

  canvas_->glVertexAttribPointer(aColor, 4, GL_FLOAT, GL_FALSE, sizeof(CGLColor), nullptr);
  canvas_->glEnableVertexAttribArray(aColor);

  //---

  assert(texCoords_.size() == np);

  uint aTexCoord = 2;
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, texCoordBufferId_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(CGLVector2D), &texCoords_[0], GL_STATIC_DRAW);

  canvas_->glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(CGLVector2D), nullptr);
  canvas_->glEnableVertexAttribArray(aTexCoord);

  //---

  canvas_->glBindBuffer(GL_ARRAY_BUFFER, 0);

  canvas_->glBindVertexArray(0);
}

void
Plane3DObj::
render()
{
  initShader();

  updateGL();

  //---

  s_program->bind();

  s_program->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  s_program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  setModelMatrix();
  s_program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix()));

  //---

  canvas_->glBindVertexArray(vertexArrayId_);

  //---

  useTexture_ = (! canvas_->isWireframe() && !!texture_);

  s_program->setUniformValue("useTexture", useTexture_);
  s_program->setUniformValue("textureId", 0);

  if (useTexture_)
    glEnable(GL_TEXTURE_2D);

  if (useTexture_) {
    glActiveTexture(GL_TEXTURE0);
    texture_->bind();
  }

  //---

  int np = points_.size();

  if (canvas_->isWireframe()) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glDrawArrays(GL_TRIANGLES, 0, np);
  }
  else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glDrawArrays(GL_TRIANGLES, 0, np);
  }

  //---

  if (useTexture_)
    glDisable(GL_TEXTURE_2D);
}

}
