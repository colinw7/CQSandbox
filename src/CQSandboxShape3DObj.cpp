#include <CQSandboxShape3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxBBox3DObj.h>
#include <CQSandboxLight3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQGLTexture.h>
#include <CQGLBuffer.h>
#include <CQGLUtil.h>
#include <CShape3D.h>
#include <CLine3D.h>

namespace CQSandbox {

ShaderProgram* Shape3DObj::s_program = nullptr;
Shape3DObjMgr* Shape3DObj::s_objectMgr;

//---

void
Shape3DObjMgr::
initRender(Canvas3D *canvas)
{
  Shape3DObj::initDraw(canvas);
}

void
Shape3DObjMgr::
termRender(Canvas3D *)
{
}

//---

Object3D *
Shape3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->tcl();

  auto *obj = new Shape3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

Shape3DObj::
Shape3DObj(Canvas3D *canvas) :
 Object3D(canvas, Type::SHAPE)
{
  if (! s_objectMgr) {
    s_objectMgr = new Shape3DObjMgr;

    canvas->addObjectMgr(s_objectMgr);
  }

  s_objectMgr->addObject(this);
}

void
Shape3DObj::
init()
{
  Object3D::init();

  //---

  initShader(canvas_);

  buffer_ = s_program->createBuffer();
}

void
Shape3DObj::
initShader(Canvas3D *canvas)
{
  if (s_program)
    return;

  auto *app = canvas->app();

  s_program = new ShaderProgram(canvas);

  s_program->addVertexFile  (app->buildDir() + "/shaders/shape.vs");
  s_program->addFragmentFile(app->buildDir() + "/shaders/shape.fs");

  s_program->link();
}

bool
Shape3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  return Object3D::getValue(name, args, value);
}

bool
Shape3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas_->app();
  auto *tcl = canvas()->tcl();

  if      (name == "points") {
    std::vector<CVector3D> points;
    if (! Util::stringToVectors3D(tcl, value, points))
      return false;

    shapeData_.setPoints(points);

    setNeedsUpdate();
  }
  else if (name == "indices") {
    shapeData_.setIndices(Util::stringToUIntArray(tcl, value));

    setNeedsUpdate();
  }
  else if (name == "colors") {
    colors_ = Util::stringToColors(tcl, value);

    setNeedsUpdate();
  }
  else if (name == "tex_coords") {
    shapeData_.setTexCoords(Util::stringToVectors2D(tcl, value));

    setNeedsUpdate();
  }
  else if (name == "color") {
    setColor(Util::stringToGLColor(tcl, value));

    setNeedsUpdate();
  }
  else if (name == "texture") {
    setTextureFile(value);

    setNeedsUpdate();
  }
  else if (name == "normal_texture") {
    setNormalTexture(value);

    setNeedsUpdate();
  }
  else if (name == "angle") {
    CPoint3D p;
    if (! Util::stringToPoint3D(tcl, value, p))
      return false;

    xAngle_ = p.getX();
    yAngle_ = p.getY();
    zAngle_ = p.getZ();

    setNeedsUpdate();
  }
  else if (name == "wireframe") {
    wireframe_ = Util::stringToBool(value);

    setNeedsUpdate();
  }

  // cone <r> <h>
  else if (name == "cone") {
    shapeType_ = ShapeType::CONE;

    QStringList strs;
    (void) tcl->splitList(value, strs);

    double r = 1.0;
    double h = 1.0;

    if      (strs.size() == 1) {
      r = Util::stringToReal(value);
      h = r;
    }
    else if (strs.size() == 2) {
      r = Util::stringToReal(strs[0]);
      h = Util::stringToReal(strs[1]);
    }
    else if (! strs.empty())
      return app->errorMsg("Invalid dimensions for cone");

    shapeData_.addCone(r, h);

    setNeedsUpdate();
  }
  // cube <sx> <sy> >sz>
  else if (name == "cube") {
    shapeType_ = ShapeType::CUBE;

    QStringList strs;
    (void) tcl->splitList(value, strs);

    double sx = 1.0, sy = 1.0, sz = 1.0;

    if      (strs.size() == 1) {
      sx = Util::stringToReal(value);
      sy = sx;
      sz = sx;
    }
    else if (strs.size() == 3) {
      sx = Util::stringToReal(strs[0]);
      sy = Util::stringToReal(strs[1]);
      sz = Util::stringToReal(strs[2]);
    }
    else if (! strs.empty())
      return app->errorMsg("bad sizes for cube");

    shapeData_.addCube(sx, sy, sz);

    setNeedsUpdate();
  }
  // cylinder <r> <h>
  else if (name == "cylinder") {
    shapeType_ = ShapeType::CYLINDER;

    QStringList strs;
    (void) tcl->splitList(value, strs);

    double r = 1.0;
    double h = 1.0;

    if      (strs.size() == 1) {
      r = Util::stringToReal(value);
      h = r;
    }
    else if (strs.size() == 2) {
      r = Util::stringToReal(strs[0]);
      h = Util::stringToReal(strs[1]);
    }
    else if (! strs.empty())
      return app->errorMsg("Invalid dimensions for cylinder");

    shapeData_.addCylinder(r, h);

    setNeedsUpdate();
  }
  // sphere <r>
  else if (name == "sphere") {
    shapeType_ = ShapeType::SPHERE;

    double r = 1.0;

    if (value != "")
      r = Util::stringToReal(value);

    shapeData_.addSphere(r);

    setNeedsUpdate();
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
Shape3DObj::
addCube(double sx, double sy, double sz)
{
  shapeData_.addCube(sx, sy, sz);

  setNeedsUpdate();
}

void
Shape3DObj::
setTextureFile(const QString &filename)
{
  textureFile_ = filename;

  if (textureFile_ != "") {
    diffuseTexture_ = new CQGLTexture;

    if (! diffuseTexture_->load(textureFile_, /*flip*/true)) {
      delete diffuseTexture_;
      diffuseTexture_ = nullptr;
    }
  }
  else {
    delete diffuseTexture_;
    diffuseTexture_ = nullptr;
  }
}

void
Shape3DObj::
setNormalTexture(const QString &filename)
{
  normalTexture_ = new CQGLTexture;

  if (! normalTexture_->load(filename, /*flip*/true)) {
    delete normalTexture_;
    normalTexture_ = nullptr;
  }
}

bool
Shape3DObj::
intersect(const CVector3D &p1, const CVector3D &p2, CPoint3D &pi1, CPoint3D &pi2) const
{
  if (! shapeData_.geom())
    return false;

  CLine3D line(p1.getX(), p1.getY(), p1.getZ(), p2.getX(), p2.getY(), p2.getZ());

  double tmin, tmax;
  if (! shapeData_.geom()->intersect(line, &tmin, &tmax))
    return false;

//if ((tmin < 0.0 || tmin > 1.0) && (tmax < 0.0 || tmax > 1.0))
//  return false;

  pi1 = line.interp(tmin);
  pi2 = line.interp(tmax);

//std::cerr << "Intersect: " << id().toStdString() << " " << tmin << " " <<  tmax << "\n";
//std::cerr << "  " << Util::point3DToString(pi1).toStdString() << " " <<
//                     Util::point3DToString(pi2).toStdString() << "\n";

  return true;
}

void
Shape3DObj::
updateGL()
{
  if (! needsUpdate_)
    return;

  needsUpdate_ = false;

  //---

  calcBBox();

  calcNormals();

  //---

  const auto &points  = shapeData_.points();
  const auto &indices = shapeData_.indices();
  const auto &normals = shapeData_.normals();

  auto np = points.size();
  auto ni = indices.size();

  assert(normals.size() == np);

  //---

  const auto &texCoords = shapeData_.texCoords();

  auto nt = texCoords.size();

  static Shape3DData::TexCoords s_texCoords;

  if (nt != np) {
    if (s_texCoords.size() != np) {
      s_texCoords.resize(np);

      for (uint i = 0; i < np; ++i)
        s_texCoords[i] = CVector2D(0, 0);
    }
  }

  auto nc = colors_.size();

  static Colors s_colors;

  if (nc != np) {
    if (s_colors.size() != np)
      s_colors.resize(np);

    auto c = this->color();

    if (isInside())
      c = CGLColor(0.8, 0.4, 0.4, 0.5);

    for (uint i = 0; i < np; ++i)
      s_colors[i] = c;
  }

  //---

  buffer_->clearBuffers();

  for (uint i = 0; i < np; ++i) {
    buffer_->addPoint (points [i]);
    buffer_->addNormal(normals[i]);
  }

  if (nt == np) {
    for (uint i = 0; i < np; ++i)
      buffer_->addTexturePoint(texCoords[i]);
  }
  else {
    for (uint i = 0; i < np; ++i)
      buffer_->addTexturePoint(s_texCoords[i]);
  }

  if (nc == np) {
    for (uint i = 0; i < np; ++i)
      buffer_->addColor(colors_[i]);
  }
  else {
    for (uint i = 0; i < np; ++i)
      buffer_->addColor(s_colors[i]);
  }

  if (ni > 0) {
    for (uint i = 0; i < ni; ++i)
      buffer_->addIndex(indices[i]);
  }

  buffer_->load();
}

CBBox3D
Shape3DObj::
calcBBox()
{
  if (! bboxValid_) {
    const auto &mm = modelMatrix();

    bbox_ = CBBox3D();

    auto np = shapeData_.points().size();

    for (uint i = 0; i < np; ++i) {
      const auto &p = shapeData_.points()[i];

      CPoint3D p1(p.x(), p.y(), p.z());

      CPoint3D p2;
      mm.multiplyPoint(p1, p2);

      bbox_ += CPoint3D(p2.x, p2.y, p2.z);
    }

    bboxValid_ = true;
  }

  return bbox_;
}

void
Shape3DObj::
calcNormals()
{
  auto np = shapeData_.points().size();

  if (shapeData_.normals().size() != np) {
    Shape3DData::Points normals;

    normals.resize(np);

    for (uint i = 0; i < np; ++i)
      normals[i] = CVector3D(0, 0, 1);

    shapeData_.setNormals(normals);
  }
}

const Shape3DObj::FaceDatas &
Shape3DObj::
getFaceDatas() const
{
  return shapeData_.faceDatas();
}

void
Shape3DObj::
render()
{
  if (canvas_->isShowBBox() || isSelected()) {
    calcBBox();

    createBBoxObj();
  }

  //---

  updateGL();

  //---

  setModelMatrix();
  s_program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix()));

  //---

  //buffer_->bind();
  canvas_->bindBuffer(buffer_);

  //---

  useDiffuseTexture_ = (diffuseTexture_ && buffer_->hasTexturePart());
  useNormalTexture_  = (normalTexture_  && buffer_->hasTexturePart());

  s_program->setUniformValue("useDiffuseTexture", useDiffuseTexture_);
  s_program->setUniformValue("textureId", 0);

  s_program->setUniformValue("useNormalTexture", useNormalTexture_);
  s_program->setUniformValue("normTex", 1);

  if (useDiffuseTexture_ || useNormalTexture_)
    glEnable(GL_TEXTURE_2D);

  if (useDiffuseTexture_) {
    glActiveTexture(GL_TEXTURE0);

    if (useDiffuseTexture_)
      diffuseTexture_->bind();
  }

  if (useNormalTexture_) {
    glActiveTexture(GL_TEXTURE1);

    normalTexture_->bind();
  }

  if (wireframe_ || canvas_->isWireframe()) {
    s_program->setUniformValue("isWireframe", 1);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }
  else {
    s_program->setUniformValue("isWireframe", 0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  auto np = shapeData_.points ().size();
  auto ni = shapeData_.indices().size();

  if (ni > 0)
    glDrawElements(GL_TRIANGLES, ni, GL_UNSIGNED_INT, nullptr);
  else {
    if      (shapeData_.isUseTriangleStrip())
      glDrawArrays(GL_TRIANGLE_STRIP, 0, np);
    else if (shapeData_.isUseTriangleFan())
      glDrawArrays(GL_TRIANGLE_FAN, 0, np);
    else
      glDrawArrays(GL_TRIANGLES, 0, np);
  }

  if (useDiffuseTexture_ || useNormalTexture_)
    glDisable(GL_TEXTURE_2D);

  //---

  //buffer_->unbind();
}

void
Shape3DObj::
initDraw(Canvas3D *canvas)
{
  //s_program->bind();
  canvas->bindProgram(s_program);

  //---

  canvas->setProgramMatrices(s_program);

  //---

  canvas->setProgramSimpleLight(s_program);

  //---

  canvas->setProgramLightGlobals(s_program);
}

void
Shape3DObj::
termDraw(Canvas3D *)
{
}

}
