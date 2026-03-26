#include <CQSandboxShape3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxBBox3DObj.h>
#include <CQSandboxLight3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQGLTexture.h>
#include <CQGLUtil.h>
#include <CShape3D.h>
#include <CLine3D.h>

namespace CQSandbox {

ShaderProgram *Shape3DObj::s_program = nullptr;

bool
Shape3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Shape3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Shape3DObj::
Shape3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

QVariant
Shape3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

bool
Shape3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas_->app();
  auto *tcl = app->tcl();

  if      (name == "points") {
    shapeData_.setPoints(Util::stringToVectors3D(tcl, value));

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
    setColor(Util::stringToColor(tcl, value));

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
    auto p = Util::stringToPoint3D(tcl, value);

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
    QStringList strs;
    (void) tcl->splitList(value, strs);

    if (strs.size() != 2) {
      app->errorMsg("Invalid dimensions for cone");
      return false;
    }

    double r = Util::stringToReal(strs[0]);
    double h = Util::stringToReal(strs[1]);

    shapeData_.addCone(r, h);

    setNeedsUpdate();
  }
  // cylinder <r> <h>
  else if (name == "cylinder") {
    QStringList strs;
    (void) tcl->splitList(value, strs);

    if (strs.size() != 2) {
      app->errorMsg("Invalid dimensions for cylinder");
      return false;
    }

    double r = Util::stringToReal(strs[0]);
    double h = Util::stringToReal(strs[1]);

    shapeData_.addCylinder(r, h);

    setNeedsUpdate();
  }
  // sphere <r>
  else if (name == "sphere") {
    double r = Util::stringToReal(value);

    shapeData_.addSphere(r);

    setNeedsUpdate();
  }
  // cube <sx> <sy> >sz>
  else if (name == "cube") {
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
    else {
      app->errorMsg("bad sizes for cube");
      return false;
    }

    shapeData_.addCube(sx, sy, sz);

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
Shape3DObj::
setNormalTexture(const QString &filename)
{
  normalTexture_ = new CQGLTexture;

  if (! normalTexture_->load(filename, /*flip*/true)) {
    delete normalTexture_;
    normalTexture_ = nullptr;
  }
}

void
Shape3DObj::
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
      "layout (location = 2) in vec4 aColor;\n"
      "layout (location = 3) in vec2 aTexCoord;\n"
      "uniform highp mat4 projection;\n"
      "uniform highp mat4 view;\n"
      "uniform highp mat4 model;\n"
      "out vec3 FragPos;\n"
      "out vec3 Normal;\n"
      "out vec4 Color;\n"
      "out vec2 TexCoord;\n"
      "void main() {\n"
      "  FragPos  = vec3(model * vec4(aPos, 1.0));\n"
      "  Normal   = mat3(transpose(inverse(model)))*aNormal;\n"
      "  Color    = aColor;\n"
      "  TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
      "  gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
      "}";
    static const char *fragmentShaderSource =
      "#version 330 core\n"
      "in vec3 FragPos;\n"
      "in vec3 Normal;\n"
      "in vec4 Color;\n"
      "in vec2 TexCoord;\n"
      "out vec4 FragColor;\n"
      "uniform vec3 viewPos;\n"
      "uniform vec3 lightPos;\n"
      "uniform vec3 lightColor;\n"
      "uniform float ambientStrength;\n"
      "uniform float diffuseStrength;\n"
      "uniform float specularStrength;\n"
      "uniform float shininess;\n"
      "uniform sampler2D textureId;\n"
      "uniform sampler2D normTex;\n"
      "uniform bool useDiffuseTexture;\n"
      "uniform bool useNormalTexture;\n"
      "void main() {\n"
      "  vec3 norm;\n"
      "  if (useNormalTexture) {\n"
      "    norm = texture(normTex, TexCoord).rgb;\n"
      "    norm = normalize(norm*2.0 - 1.0).rgb;\n"
      "  } else {\n"
      "    norm = normalize(Normal);\n"
      "  }\n"
      "  vec3 lightDir = normalize(lightPos - FragPos);\n"
      "  float diff = max(dot(norm, lightDir), 0.0);\n"
      "  vec4 diffuseColor = Color;\n"
      "  if (useDiffuseTexture) {\n"
      "    diffuseColor = texture(textureId, TexCoord);\n"
      "  }\n"
      "  vec3 diffuse = diffuseStrength*diff*vec3(diffuseColor);\n"
      "  vec3 ambient = ambientStrength*vec3(diffuseColor);\n"
      "  vec3 viewDir = normalize(viewPos - FragPos);\n"
      "  vec3 reflectDir = reflect(-lightDir, norm);\n"
      "  float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);\n"
      "  vec3 specColor = lightColor;\n"
      "  vec3 specular = specularStrength*spec*specColor;\n"
      "  vec3 result = ambient + diffuse + specular;\n"
      "  FragColor = vec4(result, diffuseColor.a);\n"
      "}\n";
#endif

    s_program = new ShaderProgram(this);

#if 0
    s_program->addVertexCode  (vertexShaderSource);
    s_program->addFragmentCode(fragmentShaderSource);
#else
    s_program->addVertexFile  (canvas_->buildDir() + "/shaders/shape.vs");
    s_program->addFragmentFile(canvas_->buildDir() + "/shaders/shape.fs");
#endif

    s_program->link();
  }

  //---

  canvas_->glGenVertexArrays(1, &vertexArrayId_);

  canvas_->glGenBuffers(1, &pointsBufferId_);
  canvas_->glGenBuffers(1, &normalsBufferId_);
  canvas_->glGenBuffers(1, &colorsBufferId_);
  canvas_->glGenBuffers(1, &texCoordBufferId_);
  canvas_->glGenBuffers(1, &indBufferId_);
}

bool
Shape3DObj::
intersect(const CGLVector3D &p1, const CGLVector3D &p2, CPoint3D &pi1, CPoint3D &pi2) const
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

  // bind the Vertex Array Object
  canvas_->glBindVertexArray(vertexArrayId_);

  //---

  int np = shapeData_.points().size();

  // store point data in array buffer (vec3, location 0)
  uint aPos = 0;
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, pointsBufferId_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(CGLVector3D),
                        &shapeData_.points()[0], GL_STATIC_DRAW);

  // set points attrib data and format (for current buffer)
  canvas_->glVertexAttribPointer(aPos, 3, GL_FLOAT, GL_FALSE, sizeof(CGLVector3D), nullptr);
  canvas_->glEnableVertexAttribArray(aPos);

  // store normal data in array buffer (vec3, location 1)
  uint aNormal = 1;
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, normalsBufferId_);
  canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(CGLVector3D),
                        &shapeData_.normals()[0], GL_STATIC_DRAW);

  // set normals attrib data and format (for current buffer)
  canvas_->glVertexAttribPointer(aNormal, 3, GL_FLOAT, GL_FALSE, sizeof(CGLVector3D), nullptr);
  canvas_->glEnableVertexAttribArray(aNormal);

  //---

  int nc = colors_.size();

  // store color data in array buffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, colorsBufferId_);
  if (nc > 0) {
    canvas_->glBufferData(GL_ARRAY_BUFFER, nc*sizeof(CGLColor), &colors_[0], GL_STATIC_DRAW);
  }
  else {
    static Colors s_colors_;

    if (int(s_colors_.size()) != np)
      s_colors_.resize(np);

    auto c = this->color();

    if (isInside())
      c = CGLColor(0.8, 0.4, 0.4, 0.5);

    for (int i = 0; i < np; ++i)
      s_colors_[i] = c;

    canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(CGLColor), &s_colors_[0], GL_STATIC_DRAW);
  }

  // set colors attrib data and format (for current buffer) (vec4, location 2)
  uint aColor = 2;
  canvas_->glVertexAttribPointer(aColor, 4, GL_FLOAT, GL_FALSE, sizeof(CGLColor), nullptr);
  canvas_->glEnableVertexAttribArray(aColor);

  //---

  int nt = shapeData_.texCoords().size();

  useDiffuseTexture_ = (texture_       && nt > 0);
  useNormalTexture_  = (normalTexture_ && nt > 0);

  if (isInside()) {
    useDiffuseTexture_ = false;
    useNormalTexture_  = false;
  }

  // store texture point data in array buffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, texCoordBufferId_);
  if (useDiffuseTexture_) {
    canvas_->glBufferData(GL_ARRAY_BUFFER, nt*sizeof(CGLVector2D),
                          &shapeData_.texCoords()[0], GL_STATIC_DRAW);
  }
  else {
    static Shape3DData::TexCoords s_texCoords;

    if (int(s_texCoords.size()) != np) {
      s_texCoords.resize(np);

      for (int i = 0; i < np; ++i)
        s_texCoords[i] = CGLVector2D(0, 0);
    }

    canvas_->glBufferData(GL_ARRAY_BUFFER, np*sizeof(CGLVector2D),
                          &s_texCoords[0], GL_STATIC_DRAW);
  }

  // set texture points attrib data and format (for current buffer) (vec2, location 3)
  uint aTexCoord = 3;
  canvas_->glVertexAttribPointer(aTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(CGLVector2D), nullptr);
  canvas_->glEnableVertexAttribArray(aTexCoord);

  //---

  // store index data in element buffer
  int ni = shapeData_.indices().size();

  if (ni > 0) {
    canvas_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBufferId_);
    canvas_->glBufferData(GL_ELEMENT_ARRAY_BUFFER, ni*sizeof(unsigned int),
                          &shapeData_.indices()[0], GL_STATIC_DRAW);
  }

  //---

  canvas_->glBindBuffer(GL_ARRAY_BUFFER, 0);
//canvas_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,  0);

  canvas_->glBindVertexArray(0);
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
      normals[i] = CGLVector3D(0, 0, 1);

    shapeData_.setNormals(normals);
  }
}

void
Shape3DObj::
render()
{
  if (canvas_->isBBox() || isSelected()) {
    calcBBox();

    createBBoxObj();

    bboxObj_->render();
  }

  //---

  updateGL();

  if (wireframe_ || canvas_->isWireframe())
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

  s_program->setUniformValue("useDiffuseTexture", useDiffuseTexture_);
  s_program->setUniformValue("useNormalTexture", useNormalTexture_);
  s_program->setUniformValue("textureId", 0);

  if (useDiffuseTexture_ || useNormalTexture_)
    glEnable(GL_TEXTURE_2D);

  if (useDiffuseTexture_) {
    glActiveTexture(GL_TEXTURE0);

    if (useDiffuseTexture_)
      texture_->bind();
  }

  if (useNormalTexture_) {
    glActiveTexture(GL_TEXTURE1);

    normalTexture_->bind();
  }

  int np = shapeData_.points ().size();
  int ni = shapeData_.indices().size();

  if (ni > 0)
    glDrawElements(GL_TRIANGLES, ni, GL_UNSIGNED_INT, nullptr);
  else {
    if      (useTriangleStrip_)
      glDrawArrays(GL_TRIANGLE_STRIP, 0, np);
    else if (useTriangleFan_)
      glDrawArrays(GL_TRIANGLE_FAN, 0, np);
    else
      glDrawArrays(GL_TRIANGLES, 0, np);
  }

  //canvas_->glBindVertexArray(0);

  if (useDiffuseTexture_ || useNormalTexture_)
    glDisable(GL_TEXTURE_2D);
}

}
