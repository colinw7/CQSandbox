#include <CQSandboxPointList3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxCamera.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQGLTexture.h>
#include <CQGLUtil.h>
#include <CQTclUtil.h>
#include <CQGLState.h>

namespace CQSandbox {

size_t                                  PointList3DObj::s_maxPoints = 50000;
PointList3DObj::PointListShaderProgram *PointList3DObj::s_program   = nullptr;

Object3D *
PointList3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->tcl();

  auto *obj = new PointList3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

PointList3DObj::
PointList3DObj(Canvas3D *canvas) :
 Object3D(canvas, Type::POINT_LIST)
{
}

bool
PointList3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  auto *app = canvas_->app();

  //---

  auto getPointPos = [&](int &pos) {
    if (args.size() <= 0)
      return false;

    if (! Util::stringToInt(args[0], pos))
      return false;

    if (pos < 0 || pos >= int(points_.size()))
      return false;

    return true;
  };

  auto getColorPos = [&](int &pos) {
    if (args.size() <= 0)
      return false;

    if (! Util::stringToInt(args[0], pos))
      return false;

    if (pos < 0 || pos >= int(colors_.size()))
      return false;

    return true;
  };

  //---

  if      (name == "list.size") {
    value = Util::intToString(int(points_.size()));
  }
  else if (name == "point.position") {
    if (args.size() > 0) {
      int pos;
      if (! getPointPos(pos))
        return app->errorMsg("Invalid index for position");

      value = Util::vector3DToString(points_[pos]);
    }
    else
      return false;
  }
  else if (name == "point.color") {
    if (args.size() > 0) {
      int pos;
      if (! getColorPos(pos))
        return app->errorMsg("Invalid index for color");

      value = Util::colorToString(colors_[pos]);
    }
    else
      return false;
  }
  else if (name == "point.size") {
    value = QVariant(pointSize());
  }
  else if (name == "cull_face") {
    value = QVariant(isCullFace());
  }
  else
    return Object3D::getValue(name, args, value);

  return true;
}

bool
PointList3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas_->app();
  auto *tcl = canvas()->tcl();

  //---

  auto getPointPos = [&](int &pos) {
    if (args.size() <= 0)
      return false;

    if (! Util::stringToInt(args[0], pos))
      return false;

    if (pos < 0 || pos >= int(points_.size()))
      return false;

    return true;
  };

  auto getColorPos = [&](int &pos) {
    if (args.size() <= 0)
      return false;

    if (! Util::stringToInt(args[0], pos))
      return false;

    if (pos < 0 || pos >= int(colors_.size()))
      return false;

    return true;
  };

  //---

  if      (name == "list.size") {
    auto n = Util::stringToInt(value);

    setNumPoints(n);
  }
  else if (name == "point.position") {
    // get index from args
    if (args.size() > 0) {
      int pos;
      if (! getPointPos(pos))
        return app->errorMsg("Invalid index for position");

      CPoint3D p;
      if (! Util::stringToPoint3D(tcl, value, p))
        return false;

      points_[pos] = p;
    }
    else
      return app->errorMsg("Missing index for position");

    bboxValid_ = false;
  }
  else if (name == "point.color") {
    // get index from args
    if (args.size() > 0) {
      int pos;
      if (! getColorPos(pos))
        return app->errorMsg("Invalid index for color");

      colors_[pos] = Util::stringToGLColor(tcl, value);
    }
    else
      return app->errorMsg("Missing index for color");
  }
  else if (name == "texture") {
    setTextureFile(value);
  }
  else if (name == "point.size") {
    double r;
    if (! Util::stringToReal(value, r))
      return false;

    setPointSize(r);
  }
  else if (name == "cull_face") {
    bool b;
    if (! Util::stringToBool(value, b))
      return false;

    setCullFace(b);
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

CBBox3D
PointList3DObj::
calcBBox()
{
  if (! bboxValid_) {
    if (! points_.empty()) {
      CRMinMax xrange, yrange, zrange;

      for (const auto &p : points_) {
        xrange.add(p.x());
        yrange.add(p.y());
        zrange.add(p.z());
      }

      bbox_ = CBBox3D(xrange.min(), yrange.min(), zrange.min(),
                      xrange.max(), yrange.max(), zrange.max());

      bboxValid_ = true;
    }
  }

  return bbox_;
}

void
PointList3DObj::
setPoints(const Points &points)
{
  setNumPoints(points.size());

  points_ = points;

  bboxValid_ = false;
}

void
PointList3DObj::
setNumPoints(int n)
{
  auto n1 = int(points_.size());

  if      (n > n1) {
    for (int i = 0; i < n - n1; ++i) {
      points_.emplace_back();
      colors_.emplace_back(1.0, 1.0, 1.0);
    }

    bboxValid_ = false;
  }
  else if (n < n1) {
    for (int i = 0; i < n1 - n; ++i) {
      points_.pop_back();
      colors_.pop_back();
    }

    bboxValid_ = false;
  }
}

void
PointList3DObj::
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
}

void
PointList3DObj::
init()
{
  Object3D::init();

  //---

  if (! s_program) {
    auto *app = canvas_->app();

    s_program = new PointListShaderProgram(this);

    s_program->addVertexFile  (app->buildDir() + "/shaders/point_list.vs");
    s_program->addFragmentFile(app->buildDir() + "/shaders/point_list.fs");

    s_program->link();

    // get program variables
    s_program->positionAttr = s_program->attributeLocation("position");
    Q_ASSERT(s_program->positionAttr != -1);

    s_program->centerAttr = s_program->attributeLocation("center");
    Q_ASSERT(s_program->centerAttr != -1);

    s_program->colorAttr = s_program->attributeLocation("color");
    Q_ASSERT(s_program->colorAttr != -1);

    s_program->setProjectionUniform();
    s_program->setViewUniform();
  }
}

void
PointList3DObj::
initData()
{
  if (dataInit_)
    return;

  dataInit_ = true;

  // The VBO containing the 4 vertices of the points.
  // Thanks to instancing, they will be shared by all points.
  static const GLfloat g_vertex_buffer_data[] = {
   -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
   -0.5f,  0.5f, 0.0f,
    0.5f,  0.5f, 0.0f,
  };

  canvas_->glGenBuffers(1, &billboardVertexBuffer_);
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, billboardVertexBuffer_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data),
                        g_vertex_buffer_data, GL_STATIC_DRAW);

  // The VBO containing the positions and sizes of the points
  canvas_->glGenBuffers(1, &pointsPositionBuffer_);
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, pointsPositionBuffer_);
  // Initialize with empty (null) buffer : it will be updated later, each frame.
  canvas_->glBufferData(GL_ARRAY_BUFFER, s_maxPoints*sizeof(CGLVector3D),
                        nullptr, GL_STREAM_DRAW);

  // The VBO containing the colors of the points
  canvas_->glGenBuffers(1, &pointsColorBuffer_);
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, pointsColorBuffer_);
  // Initialize with empty (null) buffer : it will be updated later, each frame.
  canvas_->glBufferData(GL_ARRAY_BUFFER, s_maxPoints*sizeof(CGLColor),
                        nullptr, GL_STREAM_DRAW);
}

void
PointList3DObj::
render()
{
  if (canvas_->isShowBBox() || isSelected()) {
    calcBBox();

    createBBoxObj();
  }

  //---

  initData();

  //---

  bool oldCullFace = CQGLStateInst->setCullFace(isCullFace());

  s_program->bind();

  canvas_->setProgramMatrices(s_program);

  setModelMatrix();
  s_program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix()));

  //---

  auto *camera = canvas_->currentCamera();

  canvas_->setProgramCamera(s_program, camera);

  //---

  s_program->setUniformValue("pointSize", float(pointSize()));

  //---

  bool useTexture = !!texture_;

  s_program->setUniformValue("useTexture", useTexture);
  s_program->setUniformValue("textureId", 0);

  if (useTexture) {
    CQGLStateInst->setActiveTextureNum(0, true);
    texture_->bind();
  }

  //---

  CQGLStateInst->setPolygonMode(GL_FILL);

  //---

  auto n = points_.size();

  // Update the buffers that OpenGL uses for rendering.
  // There are much more sophisticated means to stream data from the CPU to the GPU,
  // but this is outside the scope of this tutorial.
  // http://www.opengl.org/wiki/Buffer_Object_Streaming
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, pointsPositionBuffer_);
  // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  canvas_->glBufferData(GL_ARRAY_BUFFER, s_maxPoints*sizeof(CGLVector3D),
                        nullptr, GL_STREAM_DRAW);
  canvas_->glBufferSubData(GL_ARRAY_BUFFER, 0, n*sizeof(CGLVector3D), &points_[0]);

  canvas_->glBindBuffer(GL_ARRAY_BUFFER, pointsColorBuffer_);
  // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  canvas_->glBufferData(GL_ARRAY_BUFFER, s_maxPoints*sizeof(CGLColor),
                        nullptr, GL_STREAM_DRAW);
  canvas_->glBufferSubData(GL_ARRAY_BUFFER, 0, n*sizeof(CGLColor), &colors_[0]);

  // 1st attribute buffer : vertices
  canvas_->glEnableVertexAttribArray(s_program->positionAttr);
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, billboardVertexBuffer_);
  canvas_->glVertexAttribPointer(
   s_program->positionAttr,
   3,
   GL_FLOAT, // type
   GL_FALSE, // normalized?
   0,
   nullptr
  );

  // 2nd attribute buffer : positions of points' centers
  canvas_->glEnableVertexAttribArray(s_program->centerAttr);
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, pointsPositionBuffer_);
  canvas_->glVertexAttribPointer(
   s_program->centerAttr,
   3,
   GL_FLOAT, // type
   GL_FALSE, // normalized?
   0,
   nullptr
  );

  // 3rd attribute buffer : points' colors
  canvas_->glEnableVertexAttribArray(s_program->colorAttr);
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, pointsColorBuffer_);
  canvas_->glVertexAttribPointer(
   s_program->colorAttr,
   4,
   GL_FLOAT, // type
   GL_FALSE, // normalized?
   0,
   nullptr
  );

  canvas_->glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, n);

  // These functions are specific to glDrawArrays*Instanced*.
  // The first parameter is the attribute buffer we're talking about.
  // The second parameter is the "rate at which generic vertex attributes advance when
  // rendering multiple instances"
  // http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml

  // points vertices : always reuse the same 4 vertices -> 0
  canvas_->glVertexAttribDivisor(s_program->positionAttr, 0);
  // center per quad -> 1
  canvas_->glVertexAttribDivisor(s_program->centerAttr, 1);
  // color per quad -> 1
  canvas_->glVertexAttribDivisor(s_program->colorAttr, 1);

  // Draw the points !
  // This draws many times a small triangle_strip (which looks like a quad).
  // This is equivalent to :
  // for (i in n) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4),
  // but faster.
  canvas_->glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, n);

  //---

  CQGLStateInst->setCullFace(oldCullFace);

  //---

  s_program->release();
}

}
