#include <CQSandboxSurface3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxLight3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQTclUtil.h>
#include <CQGLUtil.h>

#ifdef CQSANDBOX_WATER_SURFACE
#include <CWaterSurface.h>
#endif

#ifdef CQSANDBOX_FLAG
#include <CFlag.h>
#endif

namespace CQSandbox {

ShaderProgram *Surface3DObj::s_program   = nullptr;

bool
Surface3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Surface3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Surface3DObj::
Surface3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

QVariant
Surface3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

bool
Surface3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas_->app();
  auto *tcl = app->tcl();

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

      auto z = Util::stringToReal(value);

      points_[ixy].setZ(z);
    }
    else
      app->errorMsg("Missing index for point");

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

      auto r = Util::stringToReal(cstrs[0]);
      auto g = Util::stringToReal(cstrs[1]);
      auto b = Util::stringToReal(cstrs[2]);

      colors_[ixy] = CGLVector3D(r, g, b);
    }
    else
      app->errorMsg("Missing index for color");

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

  int ii = 0;

  for (int iy = 0; iy < ny_ - 1; ++iy) {
    for (int ix = 0; ix < nx_ - 1; ++ix) {
      int ixy = iy*nx_ + ix;

      indices_[ii++] = ixy;
      indices_[ii++] = ixy + 1;
      indices_[ii++] = ixy + nx_;

      indices_[ii++] = ixy + 1;
      indices_[ii++] = ixy + nx_ + 1;
      indices_[ii++] = ixy + nx_;
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

  if (! s_program) {
#if 0
    static const char *vertexShaderSource =
      "#version 330 core\n"
      "layout (location = 0) in vec3 aPos;\n"
      "layout (location = 1) in vec3 aNormal;\n"
      "layout (location = 2) in vec3 aColor;\n"
      "uniform highp mat4 projection;\n"
      "uniform highp mat4 view;\n"
      "uniform highp mat4 model;\n"
      "out vec3 FragPos;\n"
      "out vec3 Normal;\n"
      "out vec3 Color;\n"
      "void main() {\n"
      "  FragPos = vec3(model * vec4(aPos, 1.0));\n"
      "  Normal  = mat3(transpose(inverse(model)))*aNormal;\n"
      "  Color   = aColor;\n"
      "  gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
      "}";
    static const char *fragmentShaderSource =
      "#version 330 core\n"
      "in vec3 FragPos;\n"
      "in vec3 Normal;\n"
      "in vec3 Color;\n"
      "out vec4 FragColor;\n"
      "uniform vec3 viewPos;\n"
      "uniform vec3 lightPos;\n"
      "uniform vec3 lightColor;\n"
      "uniform float ambientStrength;\n"
      "uniform float diffuseStrength;\n"
      "uniform float specularStrength;\n"
      "uniform float shininess;\n"
      "void main() {\n"
      "  vec3 norm = normalize(Normal);\n"
      "  vec3 lightDir = normalize(lightPos - FragPos);\n"
      "  float diff = max(dot(norm, lightDir), 0.0);\n"
      "  vec3 diffuseColor = Color;\n"
      "  vec3 diffuse = diffuseStrength*diff*diffuseColor;\n"
      "  vec3 ambient = ambientStrength*diffuseColor;\n"
      "  vec3 viewDir = normalize(viewPos - FragPos);\n"
      "  vec3 reflectDir = reflect(-lightDir, norm);\n"
      "  float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);\n"
      "  vec3 specColor = lightColor;\n"
      "  vec3 specular = specularStrength*spec*specColor;\n"
      "  vec3 result = ambient + diffuse + specular;\n"
      "  FragColor = vec4(result, 1.0f);\n"
      "}\n";
#endif

    s_program = new ShaderProgram(this);

#if 0
    s_program->addVertexCode  (vertexShaderSource);
    s_program->addFragmentCode(fragmentShaderSource);
#else
    s_program->addVertexFile  (canvas_->buildDir() + "/shaders/surface.vs");
    s_program->addFragmentFile(canvas_->buildDir() + "/shaders/surface.fs");
#endif

    s_program->link();
  }

  //---

  canvas_->glGenVertexArrays(1, &vertexArrayId_);

  canvas_->glGenBuffers(1, &pointsBufferId_);
  canvas_->glGenBuffers(1, &normalsBufferId_);
  canvas_->glGenBuffers(1, &colorsBufferId_);
  canvas_->glGenBuffers(1, &indBufferId_);
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

  // bind the Vertex Array Object
  canvas_->glBindVertexArray(vertexArrayId_);

  //---

  int np = points_.size();

  //---

  // store point data in array buffer
  uint aPos = 0;
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, pointsBufferId_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(CGLVector3D), &points_[0], GL_STATIC_DRAW);

  // set points attrib data and format (for current buffer)
  canvas_->glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, sizeof(CGLVector3D), nullptr);
  canvas_->glEnableVertexAttribArray(aPos);

  // store normal data in array buffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, normalsBufferId_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(CGLVector3D), &normals_[0], GL_STATIC_DRAW);

  // set normals attrib data and format (for current buffer)
  canvas_->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(CGLVector3D), nullptr);
  canvas_->glEnableVertexAttribArray(1);

  // store color data in array buffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, colorsBufferId_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(CGLVector3D), &colors_[0], GL_STATIC_DRAW);

  // set colors attrib data and format (for current buffer)
  canvas_->glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(CGLVector3D), nullptr);
  canvas_->glEnableVertexAttribArray(2);

  //---

  // store index data in element buffer
  int ni = indices_.size();

  if (ni > 0) {
    canvas_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufferId_);
    canvas_->glBufferData(GL_ELEMENT_ARRAY_BUFFER, ni*sizeof(unsigned int),
                          &indices_[0], GL_STATIC_DRAW);
  }

  //---

  canvas_->glBindBuffer(GL_ARRAY_BUFFER, 0);
//canvas_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,  0);

  canvas_->glBindVertexArray(0);
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

  if (wireframe_)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  //---

  auto *light = canvas_->currentLight();

  auto lightPos   = light->position();
  auto lightColor = light->color();

  s_program->bind();

  s_program->setUniformValue("viewPos", CQGLUtil::toVector(canvas_->viewPos()));

  s_program->setUniformValue("lightPos"  , CQGLUtil::toVector(lightPos));
  s_program->setUniformValue("lightColor", CQGLUtil::toVector(lightColor));

  s_program->setUniformValue("ambientStrength" , float(canvas_->ambient()));
  s_program->setUniformValue("diffuseStrength" , float(canvas_->diffuse()));
  s_program->setUniformValue("specularStrength", float(canvas_->specular()));
  s_program->setUniformValue("shininess"       , float(canvas_->shininess()));

  s_program->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  s_program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  setModelMatrix();
  s_program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix()));

  //---

  canvas_->glBindVertexArray(vertexArrayId_);

  //---

  int np = points_.size();
  int ni = indices_.size();

  if (ni > 0)
    glDrawElements(GL_TRIANGLES, ni, GL_UNSIGNED_INT, nullptr);
  else
    glDrawArrays(GL_TRIANGLES, 0, np);

  //canvas_->glBindVertexArray(0);
}

}
