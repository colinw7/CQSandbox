#include <CQSandboxShaderProgram.h>
#include <CQGLBuffer.h>

#include <iostream>

namespace CQSandbox {

ShaderProgram::
ShaderProgram(QObject *parent)
{
  program_ = new QOpenGLShaderProgram(parent);
}

CQGLBuffer *
ShaderProgram::
createBuffer() const
{
  return new CQGLBuffer(program_);
}

void
ShaderProgram::
addVertexFile(const QString &filename)
{
  if (! program_->addShaderFromSourceFile(QOpenGLShader::Vertex, filename))
    std::cerr << program_->log().toStdString() << "\n";
}

void
ShaderProgram::
addGeometryFile(const QString &filename)
{
  if (! program_->addShaderFromSourceFile(QOpenGLShader::Geometry, filename))
    std::cerr << program_->log().toStdString() << "\n";
}

void
ShaderProgram::
addFragmentFile(const QString &filename)
{
  if (! program_->addShaderFromSourceFile(QOpenGLShader::Fragment, filename))
    std::cerr << program_->log().toStdString() << "\n";
}

void
ShaderProgram::
addVertexCode(const QString &code)
{
  program_->addShaderFromSourceCode(QOpenGLShader::Vertex, code);
}

void
ShaderProgram::
addGeometryCode(const QString &code)
{
  program_->addShaderFromSourceCode(QOpenGLShader::Geometry, code);
}

void
ShaderProgram::
addFragmentCode(const QString &code)
{
  program_->addShaderFromSourceCode(QOpenGLShader::Fragment, code);
}

void
ShaderProgram::
link()
{
  program_->link();
}

void
ShaderProgram::
bind()
{
  program_->bind();
}

void
ShaderProgram::
release()
{
  program_->release();
}

}
