#include <CQSandboxPlane3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>
#include <CQGLTexture.h>
#include <CQGLBuffer.h>
#include <CQGLUtil.h>
#include <CQGLState.h>

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

  //---

  FaceData faceData;

  faceData.buffer = buffer_;
  faceData.pos    = 0;
  faceData.len    = 3;

  faceDataList_.add(faceData);

  faceData.pos += faceData.len;

  faceDataList_.add(faceData);

  //---

  PointData pointData;

  pointData.buffer = buffer_;

  std::vector<int> pointInds = { 0, 1, 2, 4 };

  for (int i = 0; i < 4; ++i) {
    pointData.ind = pointInds[i];

    pointDatas_.push_back(pointData);
  }

  //---

  LineData lineData;

  lineData.buffer = buffer_;

  auto addLine = [&](int i1, int i2) {
    lineData.ind1   = i1;
    lineData.ind2   = i2;

    lineDatas_.push_back(lineData);
  };

  addLine(0, 1);
  addLine(1, 2);
  addLine(3, 4);
  addLine(4, 5);
}

bool
Plane3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->tcl();

  if      (name == "color") {
    QColor c;
    if (! Util::stringToQColor(tcl, value, c))
      return false;

    setColor(c);
  }
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
loadBuffer()
{
  if (! isNeedsUpdate())
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

  loadBuffer();

  //---

  auto *program = shaderProgram();

  canvas_->bindProgram(program);

  setModelMatrix();
  program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix()));

  canvas_->setProgramMatrices(program, programMatrixData());

  //---

  canvas_->bindBuffer(buffer_);

  //---

  bool useTexture_ = false;

  if (canvas_->isTextured() && texture_)
    useTexture_ = true;

  program->setUniformValue("textureId", 0);

  bool oldTexture = CQGLStateInst->setEnableTexture(useTexture_);

  if (useTexture_) {
    CQGLStateInst->setActiveTextureNum(0, true);
    texture_->bind();
  }

  //---

  program->setUniformValue("wireframeColor", CQGLUtil::toVector(canvas_->wireframeColor()));
  program->setUniformValue("wireframeAlpha", float(canvas_->wireframeAlpha()));

  program->setUniformValue("selectColor", CQGLUtil::toVector(canvas_->selectColor()));
  program->setUniformValue("selectAlpha", float(canvas_->selectAlpha()));

  //---

  auto drawBuffer = [&](bool wireframe) {
    program->setUniformValue("isWireframe", wireframe);
    program->setUniformValue("isPoint"    , false);
    program->setUniformValue("isLine"     , false);

    CQGLStateInst->setPolygonMode(wireframe ? GL_LINE : GL_FILL);

    program->setUniformValue("useTexture", ! wireframe && useTexture_);

    CQGLBuffer::DrawData drawData;

    drawData.mode    = GL_TRIANGLES;
    drawData.indexed = buffer_->hasIndices();

    for (const auto &faceData : getFaceDatas()) {
      if (faceData.glMode >= 0)
        drawData.mode = faceData.glMode;

      drawData.pos = faceData.pos;
      drawData.len = faceData.len;

      buffer_->drawData(drawData);
    }
  };

  auto drawPoints = [&]() {
    if (getPointDatas().empty())
      return;

    program->setUniformValue("isWireframe", false);
    program->setUniformValue("isPoint"    , true);
    program->setUniformValue("isLine"     , false);

    CQGLBuffer::DrawData drawData;

    CQGLStateInst->setPointSize(8);

    drawData.mode    = GL_POINTS;
    drawData.indexed = buffer_->hasIndices();
    drawData.len     = 1;

    for (const auto &pointData : getPointDatas()) {
      drawData.pos = pointData.ind;

      buffer_->drawData(drawData);
    }
  };

  auto drawLines = [&]() {
    if (getLineDatas().empty())
      return;

    program->setUniformValue("isWireframe", false);
    program->setUniformValue("isPoint"    , false);
    program->setUniformValue("isLine"     , true);

    CQGLBuffer::DrawData drawData;

    CQGLStateInst->setLineWidth(8);

    drawData.mode    = GL_LINES;
    drawData.indexed = buffer_->hasIndices();
    drawData.len     = 2;

    for (const auto &lineData : getLineDatas()) {
      drawData.pos = lineData.ind1;

      buffer_->drawData(drawData);
    }
  };

  //---

  bool wireframe = canvas_->isWireframe();
  bool solid     = canvas_->isSolid() || canvas_->isTextured();

  if (wireframe) {
    drawBuffer(true);
  }

  if (solid) {
    drawBuffer(false);
  }

  if (canvas_->editType() == Canvas3D::EditType::POINT)
    drawPoints();

  if (canvas_->editType() == Canvas3D::EditType::LINE)
    drawLines();

  //---

  CQGLStateInst->setEnableTexture(oldTexture);

  //---

  canvas_->bindProgram(nullptr);

  canvas_->bindBuffer(nullptr);

  if (useTexture_)
    texture_->unbind();
}

}
