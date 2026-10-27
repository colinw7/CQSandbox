#include <CQSandboxShape3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxLight3D.h>
#include <CQSandboxModifier3DObj.h>
#include <CQSandboxPath3DObj.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQGLTexture.h>
#include <CQGLBuffer.h>
#include <CQGLState.h>
#include <CQGLUtil.h>
#include <CShape3D.h>
#include <CLine3D.h>

namespace CQSandbox {

ShaderProgram* Shape3DObj::s_program   = nullptr;
Shape3DObjMgr* Shape3DObj::s_objectMgr = nullptr;
CMatrix3DH     Shape3DObj::s_pvMatrix;

//---

void
Shape3DObjMgr::
initRender(Canvas3D *canvas)
{
  Shape3DObj::initDraw(canvas);
}

void
Shape3DObjMgr::
termRender(Canvas3D *canvas)
{
  Shape3DObj::termDraw(canvas);
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

  material_ = canvas_->createMaterial();
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
    std::vector<uint> indices;
    if (! Util::stringToUIntArray(tcl, value, indices))
      return false;

    shapeData_.setIndices(indices);

    setNeedsUpdate();
  }
  else if (name == "colors") {
    if (! Util::stringToColors(tcl, value, colors_))
      return false;

    setNeedsUpdate();
  }
  else if (name == "tex_coords") {
    std::vector<CVector2D> vectors;
    if (! Util::stringToVectors2D(tcl, value, vectors))
      return false;

    shapeData_.setTexCoords(vectors);

    setNeedsUpdate();
  }
  else if (name == "color") {
    QColor c;
    if (! Util::stringToQColor(tcl, value, c))
      return false;

    setDiffuseColor(c);

    setNeedsUpdate();
  }
  else if (name == "texture" || name == "diffuse_texture") {
    setDiffuseTextureFile(value);

    setNeedsUpdate();
  }
  else if (name == "normal_texture") {
    setNormalTextureFile(value);

    setNeedsUpdate();
  }
  else if (name == "specular_texture") {
    setSpecularTextureFile(value);

    setNeedsUpdate();
  }
  else if (name == "emission_texture") {
    setEmissionTextureFile(value);

    setNeedsUpdate();
  }
  else if (name == "wireframe") {
    bool b;
    if (! Util::stringToBool(value, b))
      return false;

    wireframe_ = b;

    setNeedsUpdate();
  }
  else if (name == "emission") {
    double r;
    if (! Util::stringToReal(value, r))
      return false;

    material_->setEmission(r);

    setNeedsUpdate();
  }
  else if (name == "specular") {
    double r;
    if (! Util::stringToReal(value, r))
      return false;

    material_->setSpecular(r);

    setNeedsUpdate();
  }
  else if (name == "shininess") {
    double r;
    if (! Util::stringToReal(value, r))
      return false;

    material_->setShininess(r);

    setNeedsUpdate();
  }
  else if (name == "transparency") {
    double r;
    if (! Util::stringToReal(value, r))
      return false;

    material_->setTransparency(r);

    setNeedsUpdate();
  }
  else if (name == "reflectivity") {
    double r;
    if (! Util::stringToReal(value, r))
      return false;

    material_->setReflectivity(r);

    setNeedsUpdate();
  }
  else if (name == "refractivity") {
    double r;
    if (! Util::stringToReal(value, r))
      return false;

    material_->setRefractivity(r);

    setNeedsUpdate();
  }

  // cone <r> <h>
  else if (name == "cone") {
    QStringList strs;
    (void) tcl->splitList(value, strs);

    double r = 1.0;
    double h = 1.0;

    if      (strs.size() == 1) {
      if (! Util::stringToReal(value, r))
        return false;

      h = r;
    }
    else if (strs.size() == 2) {
      if (! Util::stringToReal(strs[0], r) ||
          ! Util::stringToReal(strs[1], h))
        return false;
    }
    else if (! strs.empty())
      return app->errorMsg("Invalid dimensions for cone");

    addCone(r, h, configData_);
  }
  // cube <sx> <sy> >sz>
  else if (name == "cube") {
    double sx = 1.0, sy = 1.0, sz = 1.0;

    if (value != "") {
      QStringList strs;
      (void) tcl->splitList(value, strs);

      double sx = 1.0, sy = 1.0, sz = 1.0;

      if      (strs.size() == 1) {
        if (! Util::stringToReal(value, sx))
          return false;

        sy = sx;
        sz = sx;
      }
      else if (strs.size() == 3) {
        if (! Util::stringToReal(strs[0], sx) ||
            ! Util::stringToReal(strs[1], sy) ||
            ! Util::stringToReal(strs[2], sz))
          return false;
      }
      else if (! strs.empty())
        return app->errorMsg("bad sizes for cube");
    }

    addCube(sx, sy, sz);
  }
  // cylinder <r> <h>
  else if (name == "cylinder") {
    double r = 1.0;
    double h = 1.0;

    QStringList strs;
    (void) tcl->splitList(value, strs);

    if      (strs.size() == 1) {
      if (! Util::stringToReal(value, r))
        return false;

      h = r;
    }
    else if (strs.size() == 2) {
      if (! Util::stringToReal(strs[0], r) ||
          ! Util::stringToReal(strs[1], h))
        return false;
    }
    else if (! strs.empty())
      return app->errorMsg("Invalid dimensions for cylinder");

    addCylinder(r, h, configData_);
  }
  // sphere <r>
  else if (name == "sphere") {
    double r = 1.0;

    if (value != "") {
      if (! Util::stringToReal(value, r))
        return false;
    }

    addSphere(r, configData_);
  }
  // torus <r0> <r1> <power1> <power2>
  else if (name == "torus") {
    double r0     = 1.0;
    double r1     = 1.0;
    double power1 = 1.0;
    double power2 = 1.0;

    QStringList strs;
    (void) tcl->splitList(value, strs);

    if (strs.size() == 4) {
      if (! Util::stringToReal(strs[0], r0) ||
          ! Util::stringToReal(strs[1], r1) ||
          ! Util::stringToReal(strs[2], power1) ||
          ! Util::stringToReal(strs[3], power2))
        return false;
    }
    else
      return false;

    addTorus(r0, r1, power1, power2, configData_);
  }
  // gear <r> <n> <ts> <rot>
  else if (name == "gear") {
    double r   = 1.0;
    int    n   = 10;
    double ts  = 5.0;
    double rot = 0.0;

    QStringList strs;
    (void) tcl->splitList(value, strs);

    if (strs.size() == 4) {
      if (! Util::stringToReal(strs[0], r) ||
          ! Util::stringToInt (strs[1], n) ||
          ! Util::stringToReal(strs[2], ts) ||
          ! Util::stringToReal(strs[3], rot))
        return false;
    }
    else
      return false;

    addGear(r, n, ts, rot);
  }
  // path <path> <w> <h>
  else if (name == "path") {
    auto *pathObj = dynamic_cast<Path3DObj *>(canvas()->getObjectByName(value));
    if (! pathObj) return false;

    if (args.size() < 1)
      return false;

    QStringList strs;
    (void) tcl->splitList(args[0], strs);

    double w = 1, h = 1;

    if (strs.size() == 2) {
      if (! Util::stringToReal(strs[0], w) ||
          ! Util::stringToReal(strs[1], h))
        return false;
    }
    else
      return false;

    addPath(pathObj, w, h);
  }

  else if (name == "cap_type") {
    auto lstr = value.toLower();

    if      (lstr == "none")
      configData_.cap_type = Shape3DData::CapType::NONE;
    else if (lstr == "ngon")
      configData_.cap_type = Shape3DData::CapType::NGON;
    else if (lstr == "fan")
      configData_.cap_type = Shape3DData::CapType::FAN;
    else
      return false;
  }
  else if (name == "num_patches") {
    int i;
    if (! Util::stringToInt(value, i))
      return false;

    configData_.num_patches = i;
  }
  else if (name == "num_xy") {
    int i;
    if (! Util::stringToInt(value, i))
      return false;

    configData_.num_xy = i;
  }

  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
Shape3DObj::
addCone(double r, double h, const ConfigData &configData)
{
  shapeType_ = ShapeType::CONE;

  shapeData_.addCone(r, h, configData);

  shapeData_.setBuffer(buffer_);

  setNeedsUpdate();
}

void
Shape3DObj::
addCube(double sx, double sy, double sz)
{
  shapeType_ = ShapeType::CUBE;

  shapeData_.addCube(sx, sy, sz);

  shapeData_.setBuffer(buffer_);

  setNeedsUpdate();
}

void
Shape3DObj::
addCylinder(double r, double h, const ConfigData &configData)
{
  shapeType_ = ShapeType::CYLINDER;

  shapeData_.addCylinder(r, h, configData);

  shapeData_.setBuffer(buffer_);

  setNeedsUpdate();
}

void
Shape3DObj::
addSphere(double r, const ConfigData &configData)
{
  shapeType_ = ShapeType::SPHERE;

  shapeData_.addSphere(r, configData);

  shapeData_.setBuffer(buffer_);

  setNeedsUpdate();
}

void
Shape3DObj::
addTorus(double r0, double r1, double power1, double power2, const ConfigData &configData)
{
  shapeType_ = ShapeType::TORUS;

  shapeData_.addTorus(r0, r1, power1, power2, configData);

  shapeData_.setBuffer(buffer_);

  setNeedsUpdate();
}

void
Shape3DObj::
addGear(double r, int numTeeth, double toothSize, double rot)
{
  shapeType_ = ShapeType::GEAR;

  shapeData_.addGear(r, numTeeth, toothSize, rot);

  shapeData_.setBuffer(buffer_);

  setNeedsUpdate();
}

void
Shape3DObj::
addPath(Path3DObj *pathObj, double w, double h)
{
  shapeType_ = ShapeType::PATH;

  shapeData_.addPath(pathObj->path(), w, h);

  shapeData_.setBuffer(buffer_);

  setNeedsUpdate();
}

//---

void
Shape3DObj::
setDiffuseColor(const QColor &c)
{
  material_->setDiffuseColor(c);
}

//---

QString
Shape3DObj::
diffuseTextureFile() const
{
  return (material_->diffuseTexture() ? material_->diffuseTexture()->fileName() : "");
}

void
Shape3DObj::
setDiffuseTextureFile(const QString &filename)
{
  CQGLTexture *diffuseTexture { nullptr };

  if (filename != "") {
    diffuseTexture = new CQGLTexture;

    if (! diffuseTexture->load(filename, /*flip*/true)) {
      delete diffuseTexture;
      diffuseTexture = nullptr;
    }
  }
  else {
    diffuseTexture = nullptr;
  }

  material_->setDiffuseTexture(diffuseTexture);
}

void
Shape3DObj::
setDiffuseTexture(CQGLTexture *texture)
{
  material_->setDiffuseTexture(texture);
}

QString
Shape3DObj::
normalTextureFile() const
{
  return (material_->normalTexture() ? material_->normalTexture()->fileName() : "");
}

void
Shape3DObj::
setNormalTextureFile(const QString &filename)
{
  CQGLTexture *normalTexture { nullptr };

  if (filename != "") {
    normalTexture = new CQGLTexture;

    if (! normalTexture->load(filename, /*flip*/true)) {
      delete normalTexture;
      normalTexture = nullptr;
    }
  }
  else {
    normalTexture = nullptr;
  }

  material_->setNormalTexture(normalTexture);
}

void
Shape3DObj::
setNormalTexture(CQGLTexture *texture)
{
  material_->setNormalTexture(texture);
}

QString
Shape3DObj::
specularTextureFile() const
{
  return (material_->specularTexture() ? material_->specularTexture()->fileName() : "");
}

void
Shape3DObj::
setSpecularTextureFile(const QString &filename)
{
  CQGLTexture *specularTexture { nullptr };

  if (filename != "") {
    specularTexture = new CQGLTexture;

    if (! specularTexture->load(filename, /*flip*/true)) {
      delete specularTexture;
      specularTexture = nullptr;
    }
  }
  else {
    specularTexture = nullptr;
  }

  material_->setSpecularTexture(specularTexture);
}

void
Shape3DObj::
setSpecularTexture(CQGLTexture *texture)
{
  material_->setSpecularTexture(texture);
}

QString
Shape3DObj::
emissionTextureFile() const
{
  return (material_->emissionTexture() ? material_->emissionTexture()->fileName() : "");
}

void
Shape3DObj::
setEmissionTextureFile(const QString &filename)
{
  CQGLTexture *emissionTexture { nullptr };

  if (filename != "") {
    emissionTexture = new CQGLTexture;

    if (! emissionTexture->load(filename, /*flip*/true)) {
      delete emissionTexture;
      emissionTexture = nullptr;
    }
  }
  else {
    emissionTexture = nullptr;
  }

  material_->setEmissionTexture(emissionTexture);
}

void
Shape3DObj::
setEmissionTexture(CQGLTexture *texture)
{
  material_->setEmissionTexture(texture);
}

//---

bool
Shape3DObj::
intersectRay(const CVector3D &p1, const CVector3D &p2, CPoint3D &pi1, CPoint3D &pi2) const
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

//---

void
Shape3DObj::
loadBuffer()
{
  if (! isNeedsUpdate())
    return;

  needsUpdate_ = false;

  //---

  calcBBox();

  //---

  const auto &points  = shapeData_.points();
  const auto &indices = shapeData_.indices();

  auto np = points.size();
  auto ni = indices.size();

  //---

  const auto &normals = shapeData_.normals();

  auto nn = normals.size();

  if (nn != np) {
    if (s_normals_.size() != np) {
      s_normals_.resize(np);

      for (uint i = 0; i < np; ++i)
        s_normals_[i] = CVector3D(0, 1, 0);
    }
  }
  else
    s_normals_.clear();

  //---

  const auto &texCoords = shapeData_.texCoords();

  auto nt = texCoords.size();

  if (nt != np) {
    if (s_texCoords_.size() != np) {
      s_texCoords_.resize(np);

      for (uint i = 0; i < np; ++i)
        s_texCoords_[i] = CVector2D(0, 0);
    }
  }
  else
    s_texCoords_.clear();

  auto nc = colors_.size();

  if (nc != np) {
    if (s_colors_.size() != np)
      s_colors_.resize(np);

    auto c = material_->diffuseColor();

#if 0
    if (isInside())
      c = Qt::yellow;
#endif

    for (uint i = 0; i < np; ++i)
      s_colors_[i] = Util::qcolorToColor(c);
  }
  else
    s_colors_.clear();

  //---

  buffer_->clearBuffers();

  for (uint i = 0; i < np; ++i)
    buffer_->addPoint(points[i]);

  if (nn == np) {
    for (uint i = 0; i < np; ++i)
      buffer_->addNormal(normals[i]);
  }
  else {
    for (uint i = 0; i < np; ++i)
      buffer_->addNormal(s_normals_[i]);
  }

  if (nt == np) {
    for (uint i = 0; i < np; ++i)
      buffer_->addTexturePoint(texCoords[i]);
  }
  else {
    for (uint i = 0; i < np; ++i)
      buffer_->addTexturePoint(s_texCoords_[i]);
  }

  if (nc == np) {
    for (uint i = 0; i < np; ++i)
      buffer_->addColor(colors_[i]);
  }
  else {
    for (uint i = 0; i < np; ++i)
      buffer_->addColor(s_colors_[i]);
  }

  if (ni > 0) {
    for (uint i = 0; i < ni; ++i)
      buffer_->addIndex(indices[i]);
  }

  buffer_->load();

  //---

  shapeData_.setBuffer(buffer_);
}

CBBox3D
Shape3DObj::
calcBBox()
{
  if (! bboxValid_) {
    const auto &mm = modelMatrix();

    rawBBox_ = CBBox3D();
    bbox_    = CBBox3D();

    auto np = shapeData_.points().size();

    for (uint i = 0; i < np; ++i) {
      const auto &p = shapeData_.points()[i];

      auto p1 = p.point();

      rawBBox_ += p1;

      CPoint3D p2;
      mm.multiplyPoint(p1, p2);

      bbox_ += p2;
    }

    bboxValid_ = true;
  }

  return bbox_;
}

const FaceDatas &
Shape3DObj::
getFaceDatas() const
{
  return shapeData_.faceDatas();
}

const Shape3DObj::LineDatas &
Shape3DObj::
getLineDatas() const
{
  return shapeData_.lineDatas();
}

const Shape3DObj::PointDatas &
Shape3DObj::
getPointDatas() const
{
  return shapeData_.pointDatas();
}

void
Shape3DObj::
render()
{
  auto *program = shaderProgram();

  render(program);
}

void
Shape3DObj::
render(ShaderProgram *program)
{
  if (! isPseudo()) {
    if (canvas_->isShowBBox() || isSelected()) {
      calcBBox();

      createBBoxObj();
    }
  }

  //---

  loadBuffer();

  //---

  setModelMatrix();

  auto modelMatrix = this->modelMatrix();

  program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix));

  auto projectMatrix = s_pvMatrix*modelMatrix;

  //---

  canvas_->setProgramMaterial(program, material_);

  //---

  canvas_->bindBuffer(buffer_);

  //---

  auto *diffuseTexture  = material_->diffuseTexture ();
  auto *normalTexture   = material_->normalTexture  ();
  auto *specularTexture = material_->specularTexture();
  auto *emissionTexture = material_->emissionTexture();

  bool useDiffuseTexture  = (diffuseTexture  && buffer_->hasTexturePart());
  bool useNormalTexture   = (normalTexture   && buffer_->hasTexturePart());
  bool useSpecularTexture = (specularTexture && buffer_->hasTexturePart());
  bool useEmissionTexture = (emissionTexture && buffer_->hasTexturePart());

  bool oldTexture = CQGLStateInst->setEnableTexture(useDiffuseTexture || useNormalTexture);

  if (! canvas_->isTextured() || canvas_->isShowOrient()) {
    useDiffuseTexture  = false;
    useNormalTexture   = false;
    useSpecularTexture = false;
    useEmissionTexture = false;
  }

  //---

  program->setUniformValue("diffuseTexture.enabled", useDiffuseTexture);
  program->setUniformValue("diffuseTexture.texture", 0);

  if (useDiffuseTexture) {
    CQGLStateInst->setActiveTextureNum(0, true);

    diffuseTexture->bind();
  }

  program->setUniformValue("normalTexture.enabled", useNormalTexture);
  program->setUniformValue("normalTexture.texture", 1);

  if (useNormalTexture) {
    CQGLStateInst->setActiveTextureNum(1, true);

    normalTexture->bind();
  }

  program->setUniformValue("specularTexture.enabled", useSpecularTexture);
  program->setUniformValue("specularTexture.texture", 1);

  if (useSpecularTexture) {
    CQGLStateInst->setActiveTextureNum(2, true);

    specularTexture->bind();
  }

  program->setUniformValue("emissionTexture.enabled", useEmissionTexture);
  program->setUniformValue("emissionTexture.texture", 1);

  if (useEmissionTexture) {
    CQGLStateInst->setActiveTextureNum(3, true);

    emissionTexture->bind();
  }

  //---

//program->setUniformValue("isSelected", isSelected());

  program->setUniformValue("wireframeColor", CQGLUtil::toVector(canvas_->wireframeColor()));
  program->setUniformValue("wireframeAlpha", float(canvas_->wireframeAlpha()));

  program->setUniformValue("selectColor", CQGLUtil::toVector(canvas_->selectColor()));
  program->setUniformValue("selectAlpha", float(canvas_->selectAlpha()));

  program->setUniformValue("lighted", isLighted());
  program->setUniformValue("pseudo", isPseudo());

  program->setUniformValue("showOrient", canvas_->isShowOrient());

  //---

  auto drawBuffer = [&](bool wireframe) {
    program->setUniformValue("isWireframe", wireframe);
    program->setUniformValue("isPoint"    , false);
    program->setUniformValue("isLine"     , false);

    CQGLStateInst->setPolygonMode(wireframe ? GL_LINE : GL_FILL);

    CQGLBuffer::DrawData drawData;

    if      (shapeData_.isUseTriangleStrip())
      drawData.mode = GL_TRIANGLE_STRIP;
    else if (shapeData_.isUseTriangleFan())
      drawData.mode = GL_TRIANGLE_FAN;
    else
      drawData.mode = GL_TRIANGLES;

    if (buffer_->hasIndices())
      drawData.indexed = true;

    for (const auto &faceData : getFaceDatas()) {
      if (canvas_->isShowOrient()) {
        auto points = canvas_->getFacePoints(faceData, projectMatrix);

        auto orient = Util::pointsOrientation(points);

        if      (orient == CPolygonOrientation::CLOCKWISE)
          program->setUniformValue("orientColor", CQGLUtil::toVector(QColor(Qt::red)));
        else if (orient == CPolygonOrientation::ANTICLOCKWISE)
          program->setUniformValue("orientColor", CQGLUtil::toVector(QColor(Qt::green)));
        else
          program->setUniformValue("orientColor", CQGLUtil::toVector(QColor(Qt::blue)));
      }

      program->setUniformValue("useFaceNormal", ! s_normals_.empty());
      program->setUniformValue("faceNormal"   , CQGLUtil::toVector(faceData.normal));

      if (faceData.glMode >= 0)
        drawData.mode = faceData.glMode;

      drawData.pos = faceData.pos;
      drawData.len = faceData.len;

      buffer_->drawData(drawData);

      for (auto *modifier : modifiers_)
        modifier->drawData(program, buffer_, drawData, modelMatrix);
    }
  };

#if 0
  auto drawPoints = [&]() {
    if (getPointDatas().empty())
      return;

    program->setUniformValue("isWireframe", false);
    program->setUniformValue("isPoint"    , true);
    program->setUniformValue("isLine"     , false);

    CQGLBuffer::DrawData drawData;

    CQGLStateInst->setPointSize(8);

    program->setUniformValue("isPoint", true);

    drawData.mode    = GL_POINTS;
    drawData.indexed = false;
    drawData.len     = 1;

    for (const auto &pointData : getPointDatas()) {
      drawData.pos = pointData.ind;

      buffer_->drawData(drawData);
    }
  };
#endif

#if 0
  auto drawLines = [&]() {
    if (getLineDatas().empty())
      return;

    program->setUniformValue("isWireframe", false);
    program->setUniformValue("isPoint"    , false);
    program->setUniformValue("isLine"     , true);

    CQGLBuffer::DrawData drawData;

    CQGLStateInst->setLineWidth(4);

    program->setUniformValue("isPoint", true);

    drawData.mode = GL_LINES;
    drawData.len  = 2;

    if (buffer_->hasIndices())
      drawData.indexed = true;

    for (const auto &lineData : getLineDatas()) {
      drawData.pos = lineData.ind1;

      buffer_->drawData(drawData);
    }
  };
#endif

  //---

  program->setUniformValue("isInside", false);

  bool wireframe = false, solid = true;

  if (! isPseudo()) {
    wireframe = (wireframe_ || canvas_->isWireframe());
    solid     = (! wireframe_ && (canvas_->isSolid() || canvas_->isTextured()));
  }

  if (wireframe) {
    drawBuffer(true);
  }

  if (solid) {
    drawBuffer(false);
  }

  if (canvas_->type() == Canvas3D::Type::MODEL) {
#if 0
    if (canvas_->editType() == Canvas3D::EditType::POINT)
      drawPoints();
#endif

#if 0
    if (canvas_->editType() == Canvas3D::EditType::LINE)
      drawLines();
#endif
  }

  if (isInside()) {
    program->setUniformValue("isInside", isInside());

    drawBuffer(true);
  }

  //---

  CQGLStateInst->setEnableTexture(oldTexture);

  //---

  canvas_->bindBuffer(nullptr);
}

void
Shape3DObj::
initDraw(Canvas3D *canvas)
{
  auto *program = shaderProgram();

  initDraw(canvas, program);
}

void
Shape3DObj::
initDraw(Canvas3D *canvas, ShaderProgram *program)
{
  canvas->bindProgram(program);

  //---

  canvas->setProgramMatrices(program);

  canvas->setProgramSkybox(program, 5);

  //---

  canvas->setProgramSimpleLight(program);

  //---

  canvas->setProgramLightGlobals(program);

  //---

  auto *camera = canvas->currentCamera();

  const auto &projectionMatrix = camera->perspectiveMatrix();
  const auto &viewMatrix       = camera->viewMatrix();

  s_pvMatrix = projectionMatrix*viewMatrix;
}

void
Shape3DObj::
termDraw(Canvas3D *canvas)
{
  canvas->bindProgram(nullptr);
}

//---

bool
Shape3DObj::
setPointPosition(int ind, const CPoint3D &p)
{
  if (! Object3D::setPointPosition(ind, p))
    return false;

  shapeData_.setPoint(ind, p);

  return true;
}

//---

void
Shape3DObj::
applyMatrix(const CMatrix3DH &m)
{
  shapeData_.transform(m);

  setNeedsUpdate();
}

}
