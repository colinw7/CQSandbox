#include <CQSandboxShaderToy3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxCamera.h>
#include <CQSandboxApp.h>
#include <CQSandboxShaderToyProgram.h>

#include <CQTclUtil.h>

namespace CQSandbox {

Object3D *
ShaderToy3DObj::
create(Canvas3D *canvas, const QStringList &)
{
  auto *tcl = canvas->tcl();

  auto *obj = new ShaderToy3DObj(canvas);

  auto name = canvas->addNewObject(obj);

  obj->init();

  tcl->setResult(name);

  return obj;
}

ShaderToy3DObj::
ShaderToy3DObj(Canvas3D *canvas) :
 Object3D(canvas, Type::SHADER_TOY)
{
}

void
ShaderToy3DObj::
init()
{
  Object3D::init();

  //---

  auto *app = canvas_->app();

  shaderToyProgram_ = new ShaderToyProgram(app);
}

bool
ShaderToy3DObj::
getValue(const QString &name, const QStringList &args, QVariant &value)
{
  return Object3D::getValue(name, args, value);
}

bool
ShaderToy3DObj::
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
ShaderToy3DObj::
tick()
{
  Object3D::tick();
}

void
ShaderToy3DObj::
loadBuffer()
{
}

void
ShaderToy3DObj::
render()
{
  shaderToyProgram_->updateShader();

  auto *program = shaderToyProgram_->program();

  canvas_->bindProgram(program);

  //---

  auto *camera = canvas_->camera();

  const auto &mouseData = canvas_->mouseData();

  ShaderToyProgram::UniformData uniformData;

  uniformData.w          = canvas_->width();
  uniformData.h          = canvas_->height();
  uniformData.elapsed    = elapsed_;
  uniformData.ticks      = ticks_;
  uniformData.mousePress = mouseData.press;
  uniformData.mouseMove  = mouseData.move1;
  uniformData.cameraPos  = camera->position().point();

  shaderToyProgram_->setShaderToyUniforms(uniformData);

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

  canvas_->bindProgram(nullptr);
}

}
