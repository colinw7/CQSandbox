#include <CQSandboxShaderShape3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxBBox3DObj.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>
#include <CQSandboxShaderToyProgram.h>

#include <CQGLTexture.h>
#include <CQGLUtil.h>
#include <CQTclUtil.h>

#include <CShape3D.h>
#include <CLine3D.h>

namespace CQSandbox {

ShaderProgram* ShaderShape3DObj::s_program;

bool
ShaderShape3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new ShaderShape3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

ShaderShape3DObj::
ShaderShape3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

void
ShaderShape3DObj::
init()
{
  Object3D::init();

  //---

  if (! s_program) {
    s_program = new ShaderProgram(this);

    s_program->addVertexFile  (canvas_->buildDir() + "/shaders/shader_shape.vs");
    s_program->addFragmentFile(canvas_->buildDir() + "/shaders/shader_shape.fs");

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

QVariant
ShaderShape3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

bool
ShaderShape3DObj::
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
  else if (name == "shader_texture") {
    QStringList strs;
    (void) tcl->splitList(value, strs);

    if (strs.length() != 2) {
      app->errorMsg("Invalid number of values shader_texture <id> <file>");
      return false;
    }

    setShaderToyTexture(strs[1]);

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
ShaderShape3DObj::
setShaderToyTexture(const QString &file)
{
  shaderToyData_.program = new ShaderToyProgram(this);
  shaderToyData_.texture = new CQGLTexture;

  shaderToyData_.texture->setFunctions(canvas_);

  shaderToyData_.program->setTexture(true);
  shaderToyData_.program->setFragmentShader(file);
}

bool
ShaderShape3DObj::
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
ShaderShape3DObj::
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

  bool useDiffuseTexture = (shaderToyData_.texture && nt > 0);

  if (isInside())
    useDiffuseTexture = false;

  // store texture point data in array buffer
  canvas_->glBindBuffer(GL_ARRAY_BUFFER, texCoordBufferId_);
  if (useDiffuseTexture) {
    canvas_->glBufferData(GL_ARRAY_BUFFER, nt*sizeof(CGLVector2D),
                          &shapeData_.texCoords()[0], GL_STATIC_DRAW);
  }
  else {
    static TexCoords s_texCoords;

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
ShaderShape3DObj::
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
ShaderShape3DObj::
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
ShaderShape3DObj::
preRender()
{
  if (shaderToyData_.program && shaderToyData_.texture) {
    shaderToyData_.program->updateShader();

    if (! shaderToyData_.texture->setTarget(shaderWidth_, shaderHeight_))
      std::cerr << "Set texture shader target failed\n";

    shaderToyData_.texture->bind();

    //---

    auto *program = shaderToyData_.program->program();

    program->bind();

    //---

    shaderToyData_.program->setShaderToyUniforms(shaderWidth_, shaderHeight_, elapsed_, ticks_);

    //---

    // draw shader to screen rect
    QVector3D vertices[] = {
      QVector3D(-1.0f, -1.0f, 1.0f),
      QVector3D( 1.0f, -1.0f, 1.0f),
      QVector3D( 1.0f,  1.0f, 1.0f),

      QVector3D( 1.0f,  1.0f, 1.0f),
      QVector3D(-1.0f,  1.0f, 1.0f),
      QVector3D(-1.0f, -1.0f, 1.0f)
    };

    int coordsLocation = program->attributeLocation("a_Coordinates");

    program->enableAttributeArray(coordsLocation);

    program->setAttributeArray(coordsLocation, vertices);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    program->disableAttributeArray(coordsLocation);

    //---

    program->release();

    //---

    shaderToyData_.texture->unbind();
  }
}

void
ShaderShape3DObj::
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

  s_program->bind();

  s_program->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  s_program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  setModelMatrix();
  s_program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix()));

  //---

  canvas_->glBindVertexArray(vertexArrayId_);

  //---

  s_program->setUniformValue("textureId", 0);

  glEnable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE0);

  if (shaderToyData_.texture)
    shaderToyData_.texture->bindBuffer();

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

  if (shaderToyData_.texture)
    shaderToyData_.texture->unbindBuffer();

  //canvas_->glBindVertexArray(0);

  glDisable(GL_TEXTURE_2D);
}

}
