#include <CQSandboxPath3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQGLBuffer.h>
#include <CQGLState.h>
#include <CQGLUtil.h>

#include <C2Bezier3D.h>
#include <C3Bezier3D.h>

namespace CQSandbox {

ShaderProgram *Path3DObj::s_program   = nullptr;
Path3DObjMgr  *Path3DObj::s_objectMgr = nullptr;

//---

void
Path3DObjMgr::
initRender(Canvas3D *canvas)
{
  Path3DObj::initShader(canvas);

  Path3DObj::initDraw(canvas);
}

void
Path3DObjMgr::
termRender(Canvas3D *canvas)
{
  Path3DObj::termDraw(canvas);
}

//---

Object3D *
Path3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->tcl();

  auto *obj = new Path3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

Path3DObj::
Path3DObj(Canvas3D *canvas) :
 Object3D(canvas, Type::PATH)
{
  if (! s_objectMgr) {
    s_objectMgr = new Path3DObjMgr;

    canvas->addObjectMgr(s_objectMgr);
  }

  s_objectMgr->addObject(this);
}

void
Path3DObj::
init()
{
  Object3D::init();

  //---

  initShader(canvas_);

  buffer_ = s_program->createBuffer();
}

void
Path3DObj::
initShader(Canvas3D *canvas)
{
  if (s_program)
    return;

  auto *app = canvas->app();

  s_program = new ShaderProgram(canvas);

  s_program->addVertexFile  (app->buildDir() + "/shaders/path.vs");
  s_program->addFragmentFile(app->buildDir() + "/shaders/path.fs");

  s_program->link();
}

void
Path3DObj::
setLine(const CVector3D &p1, const CVector3D &p2)
{
  path_.clear();

  path_.moveTo(p1.point());
  path_.lineTo(p2.point());

  updatePoints();

  setNeedsUpdate();
}

bool
Path3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if (name == "tpos") {
    if (args.size() < 1)
      return false;

    double t;
    if (! Util::stringToReal(args[0], t))
      return false;

    CPoint3D pos;
    if (! path_.calc(t, numLines_, pos))
      return false;

    value = Util::point3DToString(pos);
  }
  else
    return Object3D::getValue(name, args, value);

  return true;
}

bool
Path3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->tcl();

  if      (name == "path") {
    path_.clear();

    QStringList strs;
    (void) tcl->splitList(value, strs);

    for (const auto &str : strs) {
      QStringList strs1;
      (void) tcl->splitList(str, strs1);

      if (strs1.size() < 2)
        return false;

      if      (strs1[0] == "M") {
        CVector3D p;
        if (! Util::stringToVector3D(tcl, strs1[1], p))
          return false;

        path_.moveTo(p.point());
      }
      else if (strs1[0] == "L") {
        CVector3D p;
        if (! Util::stringToVector3D(tcl, strs1[1], p))
          return false;

        path_.lineTo(p.point());
      }
      else if (strs1[0] == "Q") {
        if (strs1.size() < 3)
          return false;

        CVector3D p1, p2;
        if (! Util::stringToVector3D(tcl, strs1[1], p1) ||
            ! Util::stringToVector3D(tcl, strs1[2], p2))
          return false;

        path_.quadTo(p1.point(), p2.point());
      }
      else if (strs1[0] == "C") {
        if (strs1.size() < 4)
          return false;

        CVector3D p1, p2, p3;
        if (! Util::stringToVector3D(tcl, strs1[1], p1) ||
            ! Util::stringToVector3D(tcl, strs1[2], p2) ||
            ! Util::stringToVector3D(tcl, strs1[3], p3))
          return false;

        path_.cubicTo(p1.point(), p2.point(), p3.point());
      }
    }

    updatePoints();

    setNeedsUpdate();
  }
  else if (name == "color") {
    if (! Util::stringToGLColor(tcl, value, color_))
      return false;
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

bool
Path3DObj::
exec(const QString &op, const QStringList &args, QVariant &res)
{
  auto *tcl = canvas()->tcl();

  if      (op == "moveTo") {
    if (args.size() < 1)
      return false;

    CVector3D p;
    if (! Util::stringToVector3D(tcl, args[0], p))
      return false;

    path_.moveTo(p.point());

    updatePoints();
  }
  else if (op == "lineTo") {
    if (args.size() < 1)
      return false;

    CVector3D p;
    if (! Util::stringToVector3D(tcl, args[0], p))
      return false;

    path_.lineTo(p.point());

    updatePoints();
  }
  else if (op == "curveTo") {
    if (args.size() < 2)
      return false;

    CVector3D p1;
    if (! Util::stringToVector3D(tcl, args[0], p1))
      return false;

    CVector3D p2;
    if (! Util::stringToVector3D(tcl, args[1], p2))
      return false;

    if (args.size() > 2) {
      CVector3D p3;
      if (! Util::stringToVector3D(tcl, args[2], p3))
        return false;

      path_.cubicTo(p1.point(), p2.point(), p3.point());
    }
    else
      path_.quadTo(p1.point(), p2.point());

    updatePoints();
  }
  else
    return Object3D::exec(op, args, res);

  return true;
}

void
Path3DObj::
updatePoints()
{
  bbox_ = CBBox3D();

  points_.clear();

  std::vector<CLine3D> lines;
  path_.toLines(numLines_, lines);

  for (const auto &line : lines) {
    points_.push_back(CVector3D(line.start()));
    points_.push_back(CVector3D(line.end  ()));
  }
}

void
Path3DObj::
updateGL()
{
  if (! needsUpdate_)
    return;

  needsUpdate_ = false;

  //---

  calcBBox();

  //---

  buffer_->clearBuffers();

  auto np = points_.size();

  for (uint i = 0; i < np; ++i) {
    buffer_->addPoint(points_[i]);
    buffer_->addColor(color_);
  }

  buffer_->load();
}

CBBox3D
Path3DObj::
calcBBox()
{
  if (! bboxValid_) {
    bbox_ = CBBox3D();

    auto np = points_.size();

    for (uint i = 0; i < np; ++i)
      bbox_ += points_[i];

    bboxValid_ = true;
  }

  return bbox_;
}

void
Path3DObj::
render()
{
  updateGL();

  //---

  setModelMatrix();
  s_program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix()));

  //---

  canvas_->bindBuffer(buffer_);

  //---

  CQGLStateInst->setPolygonMode(GL_LINE);

  buffer_->drawLines();

  //---

  canvas_->bindBuffer(nullptr);
}

void
Path3DObj::
initDraw(Canvas3D *canvas)
{
  canvas->bindProgram(s_program);

  //---

  canvas->setProgramMatrices(s_program);
}

void
Path3DObj::
termDraw(Canvas3D *canvas)
{
  canvas->bindProgram(nullptr);
}

}
