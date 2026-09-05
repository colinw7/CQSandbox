#include <CQSandboxPath3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQGLBuffer.h>
#include <CQGLUtil.h>

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
termRender(Canvas3D *)
{
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

  path_.moveTo(p1);
  path_.lineTo(p2);

  updatePoints();

  setNeedsUpdate();
}

bool
Path3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  return Object3D::getValue(name, args, value);
}

bool
Path3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *tcl = canvas()->tcl();

  if (name == "path") {
    path_.clear();

    QStringList strs;
    (void) tcl->splitList(value, strs);

    for (const auto &str : strs) {
      QStringList strs1;
      (void) tcl->splitList(str, strs1);

      if (strs1.size() < 2) continue;

      if      (strs1[0] == "M") {
        CVector3D p;
        if (! Util::stringToVector3D(tcl, strs1[1], p))
          return false;

        path_.moveTo(p);
      }
      else if (strs1[0] == "L") {
        CVector3D p;
        if (! Util::stringToVector3D(tcl, strs1[1], p))
          return false;

        path_.lineTo(p);
      }
    }

    updatePoints();

    setNeedsUpdate();
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

    path_.moveTo(p);
  }
  else if (op == "lineTo") {
    if (args.size() < 1)
      return false;

    CVector3D p;
    if (! Util::stringToVector3D(tcl, args[0], p))
      return false;

    path_.lineTo(p);
  }
#if 0
  else if (op == "curveTo") {
    if (args.size() < 2)
      return false;

    CPoint3D p1;
    if (! Util::stringToPoint3D(tcl, args[0], p1))
      return false;

    CPoint3D p2;
    if (! Util::stringToPoint3D(tcl, args[1], p2))
      return false;

    if (args.size() > 2) {
      CPoint3D p3;
      if (! Util::stringToPoint3D(tcl, args[2], p3))
        return false;

      path_.cubicTo(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
    }
    else
      path_.quadTo(p1.x, p1.y, p2.x, p2.y);
  }
#endif
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

  for (const auto &element : path_.elements())
    points_.push_back(element.pos);
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

  for (uint i = 0; i < np; ++i)
    buffer_->addPoint(points_[i]);

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

  //buffer_->bind();
  canvas_->bindBuffer(buffer_);

  //---

  auto np = points_.size();

  glDrawArrays(GL_LINES, 0, np);
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
termDraw(Canvas3D *)
{
}

}
