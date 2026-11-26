#include <CQSandboxPlane3DObj.h>
#include <CQSandboxTexture3DObj.h>
#include <CQSandboxTexture.h>
#include <CQSandboxMaterial3D.h>
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
  material_ = canvas_->createMaterial();
}

void
Plane3DObj::
init()
{
  Object3D::init();

  //---

  auto *program = shaderProgram();

  buffer_ = program->createBuffer();

  //---

  static Points points = {
    CGLVector3D(-0.5f, -0.5f, 0.0f),
    CGLVector3D( 0.5f, -0.5f, 0.0f),
    CGLVector3D( 0.5f,  0.5f, 0.0f),

    CGLVector3D( 0.5f,  0.5f, 0.0f),
    CGLVector3D(-0.5f,  0.5f, 0.0f),
    CGLVector3D(-0.5f, -0.5f, 0.0f),
  };

  points_ = points;

  setTexCoords(CPoint2D(0, 0), CPoint2D(1, 1));

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

    setDiffuseColor(c);

    setNeedsUpdate();
  }
  else if (name == "texture" || name == "diffuse_texture") {
    auto *textureObj = dynamic_cast<Texture3DObj *>(canvas()->getObjectByName(value));

    if (textureObj)
      setDiffuseTexture(textureObj->texture());
    else {
      if (! setDiffuseTextureFile(value))
        return canvas()->errorMsg("Bad diffuse texture file '" + value + "'");
    }

    setNeedsUpdate();
  }
  else if (name == "texture_coords" || name == "tex_coords") {
    std::vector<CPoint2D> points;
    if (! Util::stringToPoints2D(tcl, value, points))
      return false;

    if (points.size() != 2)
      return false;

    setTexCoords(points[0], points[1]);
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
Plane3DObj::
setTexCoords(const CPoint2D &p1, const CPoint2D &p2)
{
  if (texCoords_.empty())
    texCoords_.resize(6);

  auto x1 = p1.x; auto y1 = p1.y;
  auto x2 = p2.x; auto y2 = p2.y;

  assert(texCoords_.size() == 6);

  texCoords_[0] = CGLVector2D(x1, y1);
  texCoords_[1] = CGLVector2D(x2, y1);
  texCoords_[2] = CGLVector2D(x2, y2);

  texCoords_[3] = CGLVector2D(x2, y2);
  texCoords_[4] = CGLVector2D(x1, y2);
  texCoords_[5] = CGLVector2D(x1, y1);
}

//---

QColor
Plane3DObj::
diffuseColor() const
{
  return Util::RGBAToQColor(material_->getDiffuse());
}

void
Plane3DObj::
setDiffuseColor(const QColor &c)
{
  material_->setDiffuse(Util::QColorToRGBA(c));
}

//---

QString
Plane3DObj::
diffuseTextureFile() const
{
  if (! material_->diffuseTexture())
    return "";

  auto *glTexture = dynamic_cast<Texture *>(material_->diffuseTexture())->glTexture(canvas_);

  return glTexture->fileName();
}

bool
Plane3DObj::
setDiffuseTextureFile(const QString &filename)
{
  Texture *diffuseTexture { nullptr };

  if (! canvas_->loadTexture(filename, diffuseTexture))
    return false;

  setDiffuseTexture(diffuseTexture);

  return true;
}

Texture *
Plane3DObj::
diffuseTexture() const
{
  return static_cast<Texture *>(material_->diffuseTexture());
}

void
Plane3DObj::
setDiffuseTexture(Texture *texture)
{
  material_->setDiffuseTexture(texture);
}

//---

ShaderProgram *
Plane3DObj::
shaderProgram()
{
  if (shaderProgram_)
    return shaderProgram_;

  if (! s_program) {
    auto *app = canvas_->app();

    s_program = new ShaderProgram(app);

    s_program->addVertexFile  (app->buildDir() + "/shaders/plane.vs");
    s_program->addFragmentFile(app->buildDir() + "/shaders/plane.fs");

    s_program->link();
  }

  return s_program;
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

  for (uint i = 0; i < np; ++i) {
    buffer_->addPoint (points_[i]);
    buffer_->addNormal(CVector3D(0, 0, 1));
  }

  //---

  // set colors attrib data and format (for current buffer) (vec4, location 2)
  std::vector<CGLColor> colors1;

  if (colors_.size() != np) {
    auto c  = this->diffuseColor();
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
  const auto &mm = modelMatrix();

  bbox_ = CBBox3D();

  for (const auto &p : points_) {
    const auto &p1 = CPoint3D(p.x(), p.y(), p.z());

    rawBBox_ += p1;

    CPoint3D p2;
    mm.multiplyPoint(p1, p2);

    bbox_ += p2;
  }

  return bbox_;
}

void
Plane3DObj::
render()
{
  auto *program = shaderProgram();

  //---

  loadBuffer();

  buffer_->setProgram(program->program());

  //---

  canvas_->bindProgram(program);

  //---

  canvas_->setProgramShadow(program);

  //---

  setModelMatrix();
  program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix()));

  canvas_->setProgramMatrices(program, programMatrixData());

  canvas_->setProgramLightGlobals(program);

  canvas_->setProgramLights(program);

  program->loadUniforms();

  //---

  auto cullFace = isCullFace().value_or(canvas_->isCullFace());

  bool oldCullFace = CQGLStateInst->setCullFace(cullFace);

  //---

  canvas_->bindBuffer(buffer_);

  //---

  canvas_->setProgramState(program);

  canvas_->setProgramStateColors(program);

  canvas_->setProgramMaterial(program, material_);

  program->setUniformValue("alpha", float(diffuseColor().alphaF()));

  //---

  bool useTexture = false;

  if (canvas_->isTextured() && diffuseTexture())
    useTexture = true;

  program->setUniformValue("diffuseTexture.texture", 0);

  bool oldTexture = CQGLStateInst->setEnableTexture(useTexture);

  if (useTexture) {
    CQGLStateInst->setActiveTextureNum(0, true);
    diffuseTexture()->glTexture(canvas_)->bind();
  }

  program->setUniformValue("normalTexture.enabled"  , false);
  program->setUniformValue("normalTexture.texture"  , 1);
  program->setUniformValue("specularTexture.enabled", false);
  program->setUniformValue("specularTexture.texture", 2);
  program->setUniformValue("emissionTexture.enabled", false);
  program->setUniformValue("emissionTexture.texture", 3);

  //---

  auto drawBuffer = [&](bool wireframe) {
    program->setUniformValue("isWireframe", wireframe);
    program->setUniformValue("isPoint"    , false);
    program->setUniformValue("isLine"     , false);

    CQGLStateInst->setPolygonMode(wireframe ? GL_LINE : GL_FILL);

    program->setUniformValue("diffuseTexture.enabled", ! wireframe && useTexture);

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

    CQGLStateInst->setPointSize(canvas_->editPointSize());

    drawData.mode    = GL_POINTS;
    drawData.indexed = false;
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

    CQGLStateInst->setLineWidth(canvas_->editLineWidth());

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

  canvas_->unsetProgramShadow();

  //---

  canvas_->bindProgram(nullptr);

  canvas_->bindBuffer(nullptr);

  if (useTexture)
    diffuseTexture()->glTexture(canvas_)->unbind();

  //---

  CQGLStateInst->setCullFace(oldCullFace);
}

}
