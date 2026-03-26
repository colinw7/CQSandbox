#include <CQSandboxParticleList3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxBBox3DObj.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQGLTexture.h>
#include <CQGLUtil.h>
#include <CQTclUtil.h>
#include <CGLCamera.h>

#include <CLorenzCalc.h>

#ifdef CQSANDBOX_FLOCKING
#include <CFlocking.h>
#endif

#ifdef CQSANDBOX_FIREWORKS
#include <CFireworks.h>
#endif

namespace CQSandbox {

size_t                                        ParticleList3DObj::s_maxPoints = 50000;
ParticleList3DObj::ParticleListShaderProgram *ParticleList3DObj::s_program   = nullptr;

bool
ParticleList3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new ParticleList3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

ParticleList3DObj::
ParticleList3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

QVariant
ParticleList3DObj::
getValue(const QString &name, const QStringList &args)
{
  if      (name == "size") {
    return Util::intToString(int(points_.size()));
  }
  else if (name == "position") {
    if (args.size() > 0) {
      auto i = Util::stringToInt(args[0]);

      if (i < 0 || i >= int(points_.size()))
        return QVariant();

      return Util::vector3DToString(points_[i]);
    }
    else
      return QVariant();
  }
  else if (name == "color") {
    if (args.size() > 0) {
      auto i = Util::stringToInt(args[0]);

      if (i < 0 || i >= int(colors_.size()))
        return QVariant();

      return Util::colorToString(colors_[i]);
    }
    else
      return QVariant();
  }
  else if (name == "range") {
    calcBBox();

    QStringList strs;

    strs << QString::number(bbox_.getXMin());
    strs << QString::number(bbox_.getYMin());
    strs << QString::number(bbox_.getZMin());
    strs << QString::number(bbox_.getXMax());
    strs << QString::number(bbox_.getYMax());
    strs << QString::number(bbox_.getZMax());

    return strs.join(" ");
  }
  else if (name == "particleSize") {
    return QVariant(particleSize());
  }
  else
    return Object3D::getValue(name, args);
}

bool
ParticleList3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas_->app();
  auto *tcl = app->tcl();

  if      (name == "size") {
    auto n = Util::stringToInt(value);

    setNumPoints(n);
  }
  else if (name == "position") {
    // get index from args
    if (args.size() > 0) {
      auto i = Util::stringToInt(args[0]);

      if (i < 0 || i >= int(points_.size()))
        return false;

      points_[i] = Util::stringToPoint3D(tcl, value);
    }
    else
      app->errorMsg("Missing index for position");

    bboxValid_ = false;
  }
  else if (name == "color") {
    // get index from args
    if (args.size() > 0) {
      auto i = Util::stringToInt(args[0]);

      if (i < 0 || i >= int(colors_.size()))
        return false;

      colors_[i] = Util::stringToColor(tcl, value);
    }
    else
      app->errorMsg("Missing index for color");
  }
  else if (name == "generator") {
    int n = 10000;

    if (args.size() > 0)
      n = Util::stringToInt(args[0]);

    if (value == "lorenz") {
      CRMinMax xrange, yrange, zrange;

      setNumPoints(n);

      CLorenzCalc calc(0, n);

      int j = 0;

      for (int i = 0; i < calc.getIterationEnd(); i++) {
        double x, y, z;

        calc.nextValue(x, y, z);

        if (i >= calc.getIterationStart()) {
          points_[j] = CGLVector3D(x, y, z);

#if 0
          int i1 = int(j  - 1000*(j /1000));
          int i2 =     i1 -  100*(i1/ 100);
          int i3 =     i2 -   10*(i2/  10);

          i1 -= i2 + i3;
          i2 -= i3;

          float b = i1/1000.0;
          float g = i2/100.0;
          float r = i3/10.0;

          colors_[j] = CGLColor(r, g, b);
#endif

          xrange.add(x);
          yrange.add(y);
          zrange.add(z);

          ++j;
        }
      }

      //std::cerr << "X: " << xrange.min() << " " << xrange.max() << "\n";
      //std::cerr << "Y: " << yrange.min() << " " << yrange.max() << "\n";
      //std::cerr << "Z: " << zrange.min() << " " << zrange.max() << "\n";

      for (int i = 0; i < j; ++i) {
        auto x1 = CMathUtil::map(points_[i].x(), xrange.min(), xrange.max(), -1, 1);
        auto y1 = CMathUtil::map(points_[i].y(), yrange.min(), yrange.max(), -1, 1);
        auto z1 = CMathUtil::map(points_[i].z(), zrange.min(), zrange.max(), -1, -2);

        auto r = CMathUtil::map(points_[i].x(), xrange.min(), xrange.max(), 0, 1);
        auto g = CMathUtil::map(points_[i].y(), yrange.min(), yrange.max(), 0, 1);
        auto b = CMathUtil::map(points_[i].z(), zrange.min(), zrange.max(), 0, 1);

        colors_[i] = CGLColor(r, g, b);
        points_[i] = CGLVector3D(x1, y1, z1);
      }
    }

    bboxValid_ = false;
  }
#ifdef CQSANDBOX_FLOCKING
  else if (name == "flocking") {
    delete flocking_;
#ifdef CQSANDBOX_FIREWORKS
    delete fireworks_;
#endif

    flocking_ = new CFlocking;

    auto n = flocking_->numBoids();

    setNumPoints(n);

    updateFlocking();
  }
#endif
#ifdef CQSANDBOX_FIREWORKS
  else if (name == "fireworks") {
#ifdef CQSANDBOX_FLOCKING
    delete flocking_;
#endif
    delete fireworks_;

    fireworks_ = new CFireworks;

    auto n = 100;

    setNumPoints(n);

    updateFireworks();
  }
#endif
  else if (name == "texture") {
    setTextureFile(value);
  }
  else if (name == "particleSize") {
    auto r = Util::stringToReal(value);

    setParticleSize(r);
  }
  else
    return Object3D::setValue(name, value, args);

  return true;
}

CBBox3D
ParticleList3DObj::
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
ParticleList3DObj::
setPoints(const Points &points)
{
  setNumPoints(points.size());

  points_ = points;

  bboxValid_ = false;
}

void
ParticleList3DObj::
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
ParticleList3DObj::
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
ParticleList3DObj::
init()
{
  Object3D::init();

  //---

  if (! s_program) {
#if 0
    static const char *vertexShaderSource =
      "#version 330 core\n"
      "attribute highp vec4 position;\n"
      "attribute highp vec4 center;\n"
      "attribute lowp vec4 color;\n"
      "uniform highp mat4 projection;\n"
      "uniform highp mat4 view;\n"
      "uniform highp mat4 model;\n"
      "varying lowp vec4 col;\n"
      "varying highp vec2 texPos;\n"
      "void main() {\n"
      "  col = color;\n"
      "  texPos = position.xy + 0.5;\n"
      "  gl_Position = (projection*view*model*center) + 0.05*position;\n"
      "}\n";

    static const char *fragmentShaderSource =
      "#version 330 core\n"
      "varying lowp vec4 col;\n"
      "varying highp vec2 texPos;\n"
      "uniform bool useTexture;\n"
      "uniform sampler2D textureId;\n"
      "void main() {\n"
      "  if (useTexture) {\n"
      "    vec4 tc = texture(textureId, texPos);\n"
      "    if (tc.a < 0.1) {\n"
      "      discard;\n"
      "    }\n"
      "    gl_FragColor = col*tc;\n"
      "  } else {\n"
      "    gl_FragColor = col;\n"
      "  }\n"
      "}\n";
#endif

    s_program = new ParticleListShaderProgram(this);

#if 0
    s_program->addVertexCode  (vertexShaderSource);
    s_program->addFragmentCode(fragmentShaderSource);
#else
    //s_program->addVertexFile  (canvas_->buildDir() + "/shaders/particle_list.vs");
    //s_program->addFragmentFile(canvas_->buildDir() + "/shaders/particle_list.fs");

    s_program->addVertexFile  (canvas_->buildDir() + "/shaders/particle_list_billboard.vs");
    s_program->addFragmentFile(canvas_->buildDir() + "/shaders/particle_list_billboard.fs");
#endif

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

  // The VBO containing the 4 vertices of the particles.
  // Thanks to instancing, they will be shared by all particles.
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

  // The VBO containing the positions and sizes of the particles
  canvas_->glGenBuffers(1, &particlesPositionBuffer_);
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, particlesPositionBuffer_);
  // Initialize with empty (null) buffer : it will be updated later, each frame.
  canvas_->glBufferData(GL_ARRAY_BUFFER, s_maxPoints*sizeof(CGLVector3D),
                        nullptr, GL_STREAM_DRAW);

  // The VBO containing the colors of the particles
  canvas_->glGenBuffers(1, &particlesColorBuffer_);
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, particlesColorBuffer_);
  // Initialize with empty (null) buffer : it will be updated later, each frame.
  canvas_->glBufferData(GL_ARRAY_BUFFER, s_maxPoints*sizeof(CGLColor),
                        nullptr, GL_STREAM_DRAW);
}

void
ParticleList3DObj::
tick()
{
#ifdef CQSANDBOX_FLOCKING
  if      (flocking_) {
    flocking_->update(0.1);

    updateFlocking();

    setNeedsUpdate();
  }
#endif

#ifdef CQSANDBOX_FIREWORKS
  if (fireworks_) {
    fireworks_->step();

    fireworks_->updateParticles();

    fireworks_->updateCurrentParticles();

    updateFireworks();

    setNeedsUpdate();
  }
#endif

  Object3D::tick();
}

#ifdef CQSANDBOX_FLOCKING
void
ParticleList3DObj::
updateFlocking()
{
  double w = CFlock::getWorld().getXSize()/2;
  double h = CFlock::getWorld().getYSize()/2;
  double l = CFlock::getWorld().getZSize()/2;

  uint i = 0;

  for (auto *boid : flocking_->getBoids()) {
    auto p = boid->getPos();

    auto x = CMathUtil::map(p.getX(), -w, w, -0.9, 0.9);
    auto y = CMathUtil::map(p.getY(), -h, h, -0.9, 0.9);
    auto z = CMathUtil::map(p.getZ(), -l, l, -0.9, 0.9);

    points_[i] = CGLVector3D(x, y, z);

    auto c = boid->getFlock()->getColor();

    colors_[i] = CGLColor(c.getRed(), c.getGreen(), c.getBlue());

    ++i;
  }

  bboxValid_ = false;
}
#endif

#ifdef CQSANDBOX_FIREWORKS
void
ParticleList3DObj::
updateFireworks()
{
  const auto &particles = fireworks_->currentParticles();

  auto n = std::min(particles.size(), points_.size());

  for (uint i = 0; i < n; ++i) {
    auto *particle = particles[i];
    if (! particle) continue;

    auto p = particle->getPosition();

    auto x = CMathUtil::map(p.getX(), -100, 100, -0.9, 0.9);
    auto y = CMathUtil::map(p.getY(), 0, 200, -0.9, 0.9);
    auto z = 0.0;

    points_[i] = CGLVector3D(x, y, z);

    auto c = particle->getColor();

    colors_[i] = CGLColor(c.getRed(), c.getGreen(), c.getBlue());
  }

  bboxValid_ = false;
}
#endif

void
ParticleList3DObj::
render()
{
  if (canvas_->isBBox() || isSelected()) {
    calcBBox();

    createBBoxObj();

    bboxObj_->render();
  }

  //---

  glDisable(GL_CULL_FACE);

  s_program->bind();

  s_program->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  s_program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  setModelMatrix();
  s_program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix()));

  //---

  auto *camera = canvas_->camera();

  s_program->setUniformValue("cameraUp", CQGLUtil::toVector(camera->up()));
  s_program->setUniformValue("cameraRight", CQGLUtil::toVector(camera->right()));

  //---

  s_program->setUniformValue("particleSize", float(particleSize()));

  //---

  bool useTexture = !!texture_;

  s_program->setUniformValue("useTexture", useTexture);
  s_program->setUniformValue("textureId", 0);

  if (useTexture) {
    glActiveTexture(GL_TEXTURE0);
    texture_->bind();
  }

  //---

  auto n = points_.size();

  // Update the buffers that OpenGL uses for rendering.
  // There are much more sophisticated means to stream data from the CPU to the GPU,
  // but this is outside the scope of this tutorial.
  // http://www.opengl.org/wiki/Buffer_Object_Streaming
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, particlesPositionBuffer_);
  // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  canvas_->glBufferData(GL_ARRAY_BUFFER, s_maxPoints*sizeof(CGLVector3D),
                        nullptr, GL_STREAM_DRAW);
  canvas_->glBufferSubData(GL_ARRAY_BUFFER, 0, n*sizeof(CGLVector3D), &points_[0]);

  canvas_->glBindBuffer(GL_ARRAY_BUFFER, particlesColorBuffer_);
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

  // 2nd attribute buffer : positions of particles' centers
  canvas_->glEnableVertexAttribArray(s_program->centerAttr);
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, particlesPositionBuffer_);
  canvas_->glVertexAttribPointer(
   s_program->centerAttr,
   3,
   GL_FLOAT, // type
   GL_FALSE, // normalized?
   0,
   nullptr
  );

  // 3rd attribute buffer : particles' colors
  canvas_->glEnableVertexAttribArray(s_program->colorAttr);
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, particlesColorBuffer_);
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

  // particles vertices : always reuse the same 4 vertices -> 0
  canvas_->glVertexAttribDivisor(s_program->positionAttr, 0);
  // center per quad -> 1
  canvas_->glVertexAttribDivisor(s_program->centerAttr, 1);
  // color per quad -> 1
  canvas_->glVertexAttribDivisor(s_program->colorAttr, 1);

  // Draw the particules !
  // This draws many times a small triangle_strip (which looks like a quad).
  // This is equivalent to :
  // for (i in n) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4),
  // but faster.
  canvas_->glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, n);

  s_program->release();
}

}
