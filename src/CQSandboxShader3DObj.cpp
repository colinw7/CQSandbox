#include <CQSandboxShader3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxShaderToyProgram.h>

#include <CQTclUtil.h>

namespace CQSandbox {

Object3D *
Shader3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->tcl();

  auto *obj = new Shader3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

Shader3DObj::
Shader3DObj(Canvas3D *canvas) :
 Object3D(canvas, Type::SHADER)
{
}

void
Shader3DObj::
init()
{
  Object3D::init();

  //---

  shaderToyProgram_ = new ShaderToyProgram(this);
}

bool
Shader3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  return Object3D::getValue(name, args, value);
}

bool
Shader3DObj::
setValue(const QString &name, const QString &value, const QStringList &args)
{
  if      (name == "vertex_shader")
    shaderToyProgram_->setVertexShader(value);
  else if (name == "fragment_shader")
    shaderToyProgram_->setFragmentShader(value);
  else
    return Object3D::setValue(name, value, args);

  return true;
}

void
Shader3DObj::
tick()
{
  Object3D::tick();
}

void
Shader3DObj::
render()
{
  shaderToyProgram_->updateShader();

  auto *program = shaderToyProgram_->program();

  //program->bind();
  canvas_->bindProgram(program);

  //---

  shaderToyProgram_->setShaderToyUniforms(canvas_->width(), canvas_->height(), elapsed_, ticks_);

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

  //program->release();
}

}
