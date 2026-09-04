#include <CQSandboxPlane3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>
#include <CQGLTexture.h>
#include <CQGLBuffer.h>
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

  buffer_ = s_program->createBuffer();

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

  FaceData faceData;

  faceData.pos = 0;
  faceData.len = 3;

  faceDatas_.push_back(faceData);

  faceData.pos += faceData.len;

  faceDatas_.push_back(faceData);
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

  calcBBox();

  //---

  buffer_->clearBuffers();

  //---

  auto np = points_.size();

  for (uint i = 0; i < np; ++i)
    buffer_->addPoint(points_[i]);

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

  for (uint i = 0; i < np; ++i) {
    buffer_->addColor(colors1[i]);
  }

  //---

  assert(texCoords_.size() == np);

  for (uint i = 0; i < np; ++i) {
    buffer_->addTexturePoint(texCoords_[i]);
  }

  //---

  buffer_->load();
}

CBBox3D
Plane3DObj::
calcBBox()
{
  bbox_ = CBBox3D();

  for (const auto &p : points_) {
    bbox_ += CPoint3D(p.x(), p.y(), p.z());
  }

  return bbox_;
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

  setModelMatrix();
  s_program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix()));

  canvas_->setProgramMatrices(s_program);

  //---

  //buffer_->bind();
  canvas_->bindBuffer(buffer_);

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

  //---

  //buffer_->unbind();
}

}
