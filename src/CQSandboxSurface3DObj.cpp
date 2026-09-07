#include <CQSandboxSurface3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxLight3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQGLBuffer.h>
#include <CQGLUtil.h>
#include <CQGLState.h>
#include <CQTclUtil.h>

#ifdef CQSANDBOX_WATER_SURFACE
#include <CWaterSurface.h>
#endif

#ifdef CQSANDBOX_FLAG
#include <CFlag.h>
#endif

namespace CQSandbox {

ShaderProgram *Surface3DObj::s_program = nullptr;

Object3D *
Surface3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->tcl();

  auto *obj = new Surface3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

Surface3DObj::
Surface3DObj(Canvas3D *canvas) :
 Object3D(canvas, Type::SURFACE)
{
}

bool
Surface3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  return Object3D::getValue(name, args, value);
}

bool
Surface3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas_->app();
  auto *tcl = canvas()->tcl();

  if      (name == "size") {
    QStringList strs;
    (void) tcl->splitList(value, strs);

    if (strs.size() >= 2) {
      bool ok;
      nx_ = std::max(strs[0].toInt(&ok), 0);
      ny_ = std::max(strs[1].toInt(&ok), 0);
    }

    resizePoints();

    setNeedsUpdate();
  }
#ifdef CQSANDBOX_WATER_SURFACE
  else if (name == "water_surface") {
    bool ok;
    nx_ = std::max(value.toInt(&ok), 1);
    ny_ = nx_;

    resizePoints();

#ifdef CQSANDBOX_FLAG
    delete flag_;
    flag_ = nullptr;
#endif

    delete waterSurface_;
    waterSurface_ = new CWaterSurface(nx_);

    //---

    for (int iy = 1; iy < nx_ - 1; ++iy) {
      for (int ix = 1; ix < nx_ - 1; ++ix) {
        waterSurface_->setDampening(ix, iy, 1.0);

        waterSurface_->setZ(ix, iy, 0.0);
      }
    }

    waterSurface_->setZ(int(    nx_/4.0), int(    nx_/4.0), 1.0);
    waterSurface_->setZ(int(3.0*nx_/4.0), int(3.0*nx_/4.0), 1.0);

    //---

    updateWaterSurface();
  }
#endif
#ifdef CQSANDBOX_FLAG
  else if (name == "flag") {
    QStringList strs;
    (void) tcl->splitList(value, strs);

    if (strs.size() >= 2) {
      bool ok;
      nx_ = std::max(strs[0].toInt(&ok), 0);
      ny_ = std::max(strs[1].toInt(&ok), 0);
    }

    resizePoints();

#ifdef CQSANDBOX_WATER_SURFACE
    delete waterSurface_;
    waterSurface_ = nullptr;
#endif

    delete flag_;
    flag_ = new CFlag(-0.5, -0.5, 1, 1, nx_, ny_);

    flag_->setWind(true);
    flag_->setWindForce(3.0);

    //---

    updateFlag();
  }
#endif
  else if (name == "point") {
    int ix = -1, iy = -1;

    // get index from args
    if (args.size() > 0) {
      QStringList strs;
      (void) tcl->splitList(args[0], strs);

      if (strs.size() >= 2) {
        ix = Util::stringToInt(strs[0]);
        iy = Util::stringToInt(strs[1]);
      }

      if (ix < 0 || ix >= nx_ || iy < 0 || iy >= ny_)
        return false;

      int ixy = iy*nx_ + ix;

      double z;
      if (! Util::stringToReal(value, z))
        return false;

      points_[ixy].setZ(z);
    }
    else
      return app->errorMsg("Missing index for point");

    setNeedsUpdate();
  }
  else if (name == "color") {
    int ix = -1, iy = -1;

    // get index from args
    if (args.size() > 0) {
      QStringList strs;
      (void) tcl->splitList(args[0], strs);

      if (strs.size() >= 2) {
        ix = Util::stringToInt(strs[0]);
        iy = Util::stringToInt(strs[1]);
      }

      if (ix < 0 || ix >= nx_ || iy < 0 || iy >= ny_)
        return false;

      int ixy = iy*nx_ + ix;

      QStringList cstrs;
      (void) tcl->splitList(value, cstrs);

      double r, g, b;
      if (! Util::stringToReal(cstrs[0], r) ||
          ! Util::stringToReal(cstrs[1], g) ||
          ! Util::stringToReal(cstrs[2], b))
        return false;

      colors_[ixy] = CGLVector3D(r, g, b);
    }
    else
      return app->errorMsg("Missing index for color");

    setNeedsUpdate();
  }
  else if (name == "wireframe") {
    wireframe_ = Util::stringToBool(value);

    setNeedsUpdate();
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
Surface3DObj::
resizePoints()
{
  auto np = nx_*ny_;

  points_.resize(np);

  int ip = 0;

  for (int iy = 0; iy < ny_; ++iy) {
    auto y = CMathUtil::map(iy, 0, ny_ - 1, 0.0, 1.0);

    for (int ix = 0; ix < nx_; ++ix) {
      auto x = CMathUtil::map(ix, 0, nx_ - 1, 0.0, 1.0);

      points_[ip++] = CGLVector3D(x, y, 0.0);
    }
  }

  auto ni = std::max(6*(nx_ - 1)*(ny_ - 1), 0);

  indices_.resize(ni);

  faceDatas_.clear();

  //---

  int ii = 0;

  FaceData faceData;

  faceData.len = 3;

  for (int iy = 0; iy < ny_ - 1; ++iy) {
    for (int ix = 0; ix < nx_ - 1; ++ix) {
      int ixy = iy*nx_ + ix;

      //---

      faceData.pos = ii;

      indices_[ii++] = ixy;
      indices_[ii++] = ixy + 1;
      indices_[ii++] = ixy + nx_;

      faceDatas_.push_back(faceData);

      //---

      faceData.pos = ii;

      indices_[ii++] = ixy + 1;
      indices_[ii++] = ixy + nx_ + 1;
      indices_[ii++] = ixy + nx_;

      faceDatas_.push_back(faceData);
    }
  }

  assert(ii == ni);

  colors_.resize(np);

  for (int i = 0; i < np; ++i)
    colors_[i] = CGLVector3D(0.5, 0.7, 0.4);
}

void
Surface3DObj::
init()
{
  Object3D::init();

  //---

  initShader();

  //---

  buffer_ = s_program->createBuffer();
}

void
Surface3DObj::
initShader()
{
  if (s_program)
    return;

  auto *app = canvas_->app();

  s_program = new ShaderProgram(this);

  s_program->addVertexFile  (app->buildDir() + "/shaders/surface.vs");
  s_program->addFragmentFile(app->buildDir() + "/shaders/surface.fs");

  s_program->link();
}

void
Surface3DObj::
tick()
{
#ifdef CQSANDBOX_WATER_SURFACE
  if (waterSurface_) {
    waterSurface_->step(0.1);

    updateWaterSurface();

    setNeedsUpdate();
  }
#endif

#ifdef CQSANDBOX_FLAG
  if (flag_) {
    flag_->step(0.0005);

    updateFlag();

    setNeedsUpdate();
  }
#endif

  Object3D::tick();
}

#ifdef CQSANDBOX_WATER_SURFACE
void
Surface3DObj::
updateWaterSurface()
{
  uint nxy = nx_*ny_;

  for (uint i = 0; i < nxy; ++i) {
    points_[i].setZ(waterSurface_->getZ(i));
  }
}
#endif

#ifdef CQSANDBOX_FLAG
void
Surface3DObj::
updateFlag()
{
  int i = 0;

  for (int iy = 0; iy < ny_; ++iy) {
    for (int ix = 0; ix < nx_; ++ix) {
      auto *particle = flag_->getParticle(ix, iy);

      auto pos = particle->getPosition();

      points_[i].setX(pos.getX());
      points_[i].setY(pos.getY());
      points_[i].setZ(pos.getZ());

      ++i;
    }
  }
}
#endif

void
Surface3DObj::
updateGL()
{
  if (! needsUpdate_)
    return;

  needsUpdate_ = false;

  //---

  calcNormals();

  //---

  auto np = points_ .size();
  auto ni = indices_.size();

  //---

  buffer_->clearBuffers();

  for (uint i = 0; i < np; ++i) {
    buffer_->addPoint (points_ [i]);
    buffer_->addNormal(normals_[i]);
    buffer_->addColor (colors_ [i]);
  }

  for (uint i = 0; i < ni; ++i)
    buffer_->addIndex(indices_[i]);

  buffer_->load();
}

void
Surface3DObj::
calcNormals()
{
  auto np = points_.size();

  normals_.resize(np);

  for (int iy = 0; iy < ny_ - 1; ++iy) {
    for (int ix = 0; ix < nx_ - 1; ++ix) {
      auto ixy = iy*nx_ + ix;

      const auto &v1 = points_[ixy];
      const auto &v2 = points_[ixy + 1];
      const auto &v3 = points_[ixy + nx_];

      CGLVector3D diff1(v1, v2);
      CGLVector3D diff2(v2, v3);

      auto n = diff1.crossProduct(diff2).normalized();

      normals_[ixy          ] = n;
      normals_[ixy + 1      ] = n;
      normals_[ixy + nx_    ] = n;
      normals_[ixy + nx_ + 1] = n;
    }
  }
}

void
Surface3DObj::
render()
{
  updateGL();

  //---

  canvas_->bindProgram(s_program);

  canvas_->setProgramMatrices(s_program);

  canvas_->setProgramLightGlobals(s_program);

  canvas_->setProgramSimpleLight(s_program);

  setModelMatrix();
  s_program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix()));

  //---

  canvas_->bindBuffer(buffer_);

  //---

  bool solid     = ! wireframe_;
  bool wireframe = (wireframe_ || canvas_->isWireframe());

  if (solid) {
    s_program->setUniformValue("isWireframe", 0);

    CQGLStateInst->setPolygonMode(GL_FILL);

    if (buffer_->numIndices() > 0)
      buffer_->drawTriangleIndices();
    else
      buffer_->drawTriangles();
  }

  if (wireframe) {
    s_program->setUniformValue("isWireframe", 1);

    CQGLStateInst->setPolygonMode(GL_LINE);

    if (buffer_->numIndices() > 0)
      buffer_->drawTriangleIndices();
    else
      buffer_->drawTriangles();
  }

  canvas_->bindBuffer(nullptr);

  //---

  canvas_->bindProgram(nullptr);
}

}
