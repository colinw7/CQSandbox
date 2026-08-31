#include <CQSandboxPlane3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>
#include <CQGLTexture.h>
#include <CQGLUtil.h>

namespace CQSandbox {

ShaderProgram *Plane3DObj::s_program = nullptr;

Object3D *
Plane3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->tcl();

  auto *obj = new Plane3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

Plane3DObj::
Plane3DObj(Canvas3D *canvas) :
 Object3D(canvas, Type::PLANE)
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
  auto *tcl = canvas()->tcl();

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
    auto *app = canvas_->app();

    s_program = new ShaderProgram(this);

    s_program->addVertexFile  (app->buildDir() + "/shaders/plane.vs");
    s_program->addFragmentFile(app->buildDir() + "/shaders/plane.fs");

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

  //s_program->bind();
  canvas_->bindProgram(s_program);

  canvas_->setProgramMatrices(s_program);

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
