#include <CQSandboxLineList3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxCamera.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQGLTexture.h>
#include <CQGLBuffer.h>
#include <CQGLUtil.h>
#include <CQGLState.h>
#include <CQTclUtil.h>

namespace CQSandbox {

ShaderProgram *LineList3DObj::s_program = nullptr;

Object3D *
LineList3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->tcl();

  auto *obj = new LineList3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

LineList3DObj::
LineList3DObj(Canvas3D *canvas) :
 Object3D(canvas, Type::LINE_LIST)
{
}

bool
LineList3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  if      (name == "list.size") {
    value = Util::intToString(int(lines_.size()));
  }
  else if (name == "line.start") {
    if (args.size() > 0) {
      auto i = Util::stringToInt(args[0]);

      if (i < 0 || i >= int(lines_.size()))
        return false;

      value = Util::point3DToString(lines_[i].start());
    }
    else
      return false;
  }
  else if (name == "line.end") {
    if (args.size() > 0) {
      auto i = Util::stringToInt(args[0]);

      if (i < 0 || i >= int(lines_.size()))
        return false;

      value = Util::point3DToString(lines_[i].end());
    }
    else
      return false;
  }
  else if (name == "line.color") {
    if (args.size() > 0) {
      auto i = Util::stringToInt(args[0]);

      if (i < 0 || i >= int(colors_.size()))
        return false;

      value = Util::colorToString(colors_[i]);
    }
    else
      return false;
  }
  else if (name == "line.width") {
    value = QVariant(lineWidth());
  }
  else
    return Object3D::getValue(name, args, value);

  return true;
}

bool
LineList3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas_->app();
  auto *tcl = canvas()->tcl();

  if      (name == "list.size") {
    auto n = Util::stringToInt(value);

    setNumLines(n);
  }
  else if (name == "line.start") {
    // get index from args
    if (args.size() > 0) {
      auto i = Util::stringToInt(args[0]);

      if (i < 0 || i >= int(lines_.size()))
        return false;

      CPoint3D p;
      if (! Util::stringToPoint3D(tcl, value, p))
        return false;

      lines_[i].setStart(p);
    }
    else
      return app->errorMsg("Missing index for position");

    bboxValid_ = false;
  }
  else if (name == "line.end") {
    // get index from args
    if (args.size() > 0) {
      auto i = Util::stringToInt(args[0]);

      if (i < 0 || i >= int(lines_.size()))
        return false;

      CPoint3D p;
      if (! Util::stringToPoint3D(tcl, value, p))
        return false;

      lines_[i].setEnd(p);
    }
    else
      return app->errorMsg("Missing index for position");

    bboxValid_ = false;
  }
  else if (name == "line.color") {
    // get index from args
    if (args.size() > 0) {
      auto i = Util::stringToInt(args[0]);

      if (i < 0 || i >= int(colors_.size()))
        return app->errorMsg("Invalid index for color");

      colors_[i] = Util::stringToGLColor(tcl, value);
    }
    else
      return app->errorMsg("Missing index for color");
  }
  else if (name == "line.width") {
    double r;
    if (! Util::stringToReal(value, r))
       return false;

    setLineWidth(r);
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
LineList3DObj::
setLines(const Lines &lines)
{
  setNumLines(lines.size());

  lines_ = lines;

  needsUpdate_ = true;
  bboxValid_   = false;
}

void
LineList3DObj::
setNumLines(int n)
{
  auto n1 = int(lines_.size());
  if (n == n1) return;

  if      (n > n1) {
    for (int i = 0; i < n - n1; ++i) {
      lines_ .emplace_back();
      colors_.emplace_back(1.0, 1.0, 1.0);
    }
  }
  else {
    for (int i = 0; i < n1 - n; ++i) {
      lines_ .pop_back();
      colors_.pop_back();
    }
  }

  bboxValid_   = false;
  needsUpdate_ = true;
}

void
LineList3DObj::
init()
{
  Object3D::init();

  //---

  initShader();

  //---

  buffer_ = s_program->createBuffer();
}

void
LineList3DObj::
initShader()
{
  if (s_program)
    return;

  auto *app = canvas_->app();

  s_program = new ShaderProgram(this);

  s_program->addVertexFile  (app->buildDir() + "/shaders/line_list.vs");
  s_program->addFragmentFile(app->buildDir() + "/shaders/line_list.fs");

  s_program->link();
}

CBBox3D
LineList3DObj::
calcBBox()
{
  if (! bboxValid_) {
    if (! lines_.empty()) {
      CRMinMax xrange, yrange, zrange;

      for (const auto &l : lines_) {
        auto p1 = l.start();
        auto p2 = l.end();

        xrange.add(p1.x); yrange.add(p1.y); zrange.add(p1.z);
        xrange.add(p2.x); yrange.add(p2.y); zrange.add(p2.z);
      }

      bbox_ = CBBox3D(xrange.min(), yrange.min(), zrange.min(),
                      xrange.max(), yrange.max(), zrange.max());

      bboxValid_ = true;
    }
  }

  return bbox_;
}

void
LineList3DObj::
updateGL()
{
  if (! needsUpdate_)
    return;

  needsUpdate_ = false;

  //---

  buffer_->clearBuffers();

  uint i = 0;

  for (const auto &line : lines_) {
    auto p1 = line.start();
    auto p2 = line.end();

    const auto &c = colors_[i];

    buffer_->addPoint(p1);
    buffer_->addColor(c);

    buffer_->addPoint(p2);
    buffer_->addColor(c);

    ++i;
  }

  buffer_->load();
}

void
LineList3DObj::
render()
{
  if (canvas_->isShowBBox() || isSelected()) {
    calcBBox();

    createBBoxObj();
  }

  //---

  updateGL();

  //---

  canvas_->bindProgram(s_program);

  canvas_->setProgramMatrices(s_program);

  setModelMatrix();
  s_program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix()));

  s_program->setUniformValue("lineWidth", float(lineWidth()));

  //---

  canvas_->bindBuffer(buffer_);

  //---

  bool oldCullFace = CQGLStateInst->setCullFace(false);

  CQGLStateInst->setPolygonMode(GL_LINE);

  buffer_->drawLines();

  CQGLStateInst->setCullFace(oldCullFace);

  //---

  canvas_->bindBuffer(nullptr);

  //---

  canvas_->bindProgram(nullptr);
}

}
