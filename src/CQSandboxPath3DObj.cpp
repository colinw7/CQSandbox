#include <CQSandboxPath3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxUtil.h>

#include <CQGLUtil.h>

namespace CQSandbox {

ShaderProgram *Path3DObj::s_program = nullptr;

bool
Path3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->app()->tcl();

  auto *obj = new Path3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return true;
}

Path3DObj::
Path3DObj(Canvas3D *canvas) :
 Object3D(canvas)
{
}

void
Path3DObj::
init()
{
  Object3D::init();

  //---

  canvas_->glGenVertexArrays(1, &vertexArrayId_);

  canvas_->glGenBuffers(1, &pointsBufferId_);
}

void
Path3DObj::
initShader()
{
  if (! s_program) {
#if 0
    static const char *vertexShaderSource =
      "#version 330 core\n"
      "layout (location = 0) in vec3 aPos;\n"
      "uniform highp mat4 projection;\n"
      "uniform highp mat4 view;\n"
      "uniform highp mat4 model;\n"
      "void main() {\n"
      "  gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
      "}";
    static const char *fragmentShaderSource =
      "#version 330 core\n"
      "void main() {\n"
      "  gl_FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
      "}\n";
#endif

    s_program = new ShaderProgram(this);

#if 0
    s_program->addVertexCode  (vertexShaderSource);
    s_program->addFragmentCode(fragmentShaderSource);
#else
    s_program->addVertexFile  (canvas_->buildDir() + "/shaders/path.vs");
    s_program->addFragmentFile(canvas_->buildDir() + "/shaders/path.fs");
#endif

    s_program->link();
  }
}

void
Path3DObj::
setLine(const CGLVector3D &p1, const CGLVector3D &p2)
{
  path_.clear();

  path_.moveTo(p1);
  path_.lineTo(p2);

  updatePoints();

  setNeedsUpdate();
}

QVariant
Path3DObj::
getValue(const QString &name, const QStringList &args)
{
  return Object3D::getValue(name, args);
}

bool
Path3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  auto *app = canvas_->app();
  auto *tcl = app->tcl();

  if (name == "path") {
    path_.clear();

    QStringList strs;
    (void) tcl->splitList(value, strs);

    for (const auto &str : strs) {
      QStringList strs1;
      (void) tcl->splitList(str, strs1);

      if (strs1.size() < 2) continue;

      if      (strs1[0] == "M") {
        auto p = Util::stringToVector3D(tcl, strs1[1]);

        path_.moveTo(p);
      }
      else if (strs1[0] == "L") {
        auto p = Util::stringToVector3D(tcl, strs1[1]);

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

void
Path3DObj::
updatePoints()
{
  points_.clear();

  for (const auto &element : path_.elements()) {
    points_.push_back(element.pos);
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

  //---

  canvas_->glBindBuffer(GL_ARRAY_BUFFER, 0);
//canvas_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,  0);

  canvas_->glBindVertexArray(0);
}

void
Path3DObj::
render()
{
  initShader();

  updateGL();

  //---

  s_program->bind();

  s_program->setUniformValue("projection", CQGLUtil::toQMatrix(canvas_->projectionMatrix()));
  s_program->setUniformValue("view", CQGLUtil::toQMatrix(canvas_->viewMatrix()));

  setModelMatrix();
  s_program->setUniformValue("model", CQGLUtil::toQMatrix(modelMatrix()));

  //---

  canvas_->glBindVertexArray(vertexArrayId_);

  //---

  auto np = points_.size();

  glDrawArrays(GL_LINES, 0, np);
}

}
