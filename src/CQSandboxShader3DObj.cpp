#include <CQSandboxShader3DObj.h>
#include <CQSandboxCanvas3D.h>
#include <CQSandboxApp.h>
#include <CQSandboxShaderProgram.h>

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

  shaderProgram_ = new ShaderProgram(this);
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
  auto *tcl = canvas()->tcl();

  if      (name == "vertex_shader")
    shaderProgram_->addVertexFile(value);
  else if (name == "geometry_shader")
    shaderProgram_->addGeometryFile(value);
  else if (name == "fragment_shader")
    shaderProgram_->addFragmentFile(value);

  else if (name == "uniform_int") {
    if (args.size() != 1)
      return false;

    int i;
    if (! Util::stringToInt(args[0], i))
      return false;

    auto *uniform = new ShaderUniformT<int>(shaderProgram_, value);

    uniform->setValue(i);

    shaderProgram_->addUniform(uniform);
  }
  else if (name == "uniform_real") {
    if (args.size() != 1)
      return false;

    double r;
    if (! Util::stringToReal(args[0], r))
      return false;

    auto *uniform = new ShaderUniformT<double>(shaderProgram_, value);

    uniform->setValue(r);

    shaderProgram_->addUniform(uniform);
  }
  else if (name == "uniform_color") {
    if (args.size() != 1)
      return false;

    CRGBA c;
    if (! Util::stringToRGBA(tcl, args[0], c))
      return false;

    auto *uniform = new ShaderUniformT<CRGBA>(shaderProgram_, value);

    uniform->setValue(c);

    shaderProgram_->addUniform(uniform);
  }
  else if (name == "uniform_real_array") {
    if (args.size() != 1)
      return false;

    std::vector<double> reals;
    if (! Util::stringToRealArray(tcl, args[0], reals))
      return false;

    auto *uniform = new ShaderUniformArrayT<double>(shaderProgram_, value);

    uniform->setValue(reals);

    shaderProgram_->addUniform(uniform);
  }
  else if (name == "uniform_point2d_array") {
    if (args.size() != 1)
      return false;

    QStringList strs;
    if (! tcl->splitList(args[0], strs))
      return false;

    std::vector<CPoint2D> points;

    for (const auto  &s : strs) {
      std::vector<double> reals;
      if (! Util::stringToRealArray(tcl, s, reals))
        return false;

      if (reals.size() != 2)
        return false;

      auto p = CPoint2D(reals[0], reals[1]);

      points.push_back(p);
    }

    auto *uniform = new ShaderUniformArrayT<CPoint2D>(shaderProgram_, value);

    uniform->setValue(points);

    shaderProgram_->addUniform(uniform);
  }

  else
    return Object3D::setValue(name, value, args);

  return true;
}

bool
Shader3DObj::
exec(const QString &op, const QStringList &args, QVariant &res)
{
  if (op == "link") {
    shaderProgram_->link();
  }
  else
    return Object3D::exec(op, args, res);

  return true;
}

}
