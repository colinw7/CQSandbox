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
  auto *buffer = new CQGLBuffer(program_);

  buffer->setInd(CQGLBuffer::nextInd());

  CQGLBufferMgrInst->addBuffer(buffer);

  return buffer;
}

bool
ShaderProgram::
addVertexFile(const QString &filename)
{
#if 1
  QString code;
  if (! fileToCode(filename, code))
    return false;

  if (! addVertexCode(code, filename))
    return false;
#else
  if (! program_->addShaderFromSourceFile(QOpenGLShader::Vertex, filename)) {
    std::cerr << program_->log().toStdString() << "\n";
    return false;
  }
#endif

  vertexFile_ = filename;

  return true;
}

bool
ShaderProgram::
addGeometryFile(const QString &filename)
{
#if 1
  QString code;
  if (! fileToCode(filename, code))
    return false;

  if (! addGeometryCode(code, filename))
    return false;
#else
  if (! program_->addShaderFromSourceFile(QOpenGLShader::Fragment, filename)) {
    std::cerr << program_->log().toStdString() << "\n";
    return false;
  }
#endif

  geometryFile_ = filename;

  return true;
}

bool
ShaderProgram::
addFragmentFile(const QString &filename)
{
#if 1
  QString code;
  if (! fileToCode(filename, code))
    return false;

  if (! addFragmentCode(code, filename))
    return false;
#else
  if (! program_->addShaderFromSourceFile(QOpenGLShader::Fragment, filename)) {
    std::cerr << program_->log().toStdString() << "\n";
    return false;
  }
#endif

  fragmentFile_ = filename;

  return true;
}

bool
ShaderProgram::
addVertexCode(const QString &code, const QString &filename)
{
  if (! program_->addShaderFromSourceCode(QOpenGLShader::Vertex, code)) {
    if (filename != "")
      std::cerr << "File: '" << filename.toStdString() << "'\n";
    std::cerr << program_->log().toStdString() << "\n";
    return false;
  }

  return true;
}

bool
ShaderProgram::
addGeometryCode(const QString &code, const QString &filename)
{
  if (! program_->addShaderFromSourceCode(QOpenGLShader::Geometry, code)) {
    if (filename != "")
      std::cerr << "File: '" << filename.toStdString() << "'\n";
    std::cerr << program_->log().toStdString() << "\n";
    return false;
  }

  return true;
}

bool
ShaderProgram::
addFragmentCode(const QString &code, const QString &filename)
{
  if (! program_->addShaderFromSourceCode(QOpenGLShader::Fragment, code)) {
    if (filename != "")
      std::cerr << "File: '" << filename.toStdString() << "'\n";
    std::cerr << program_->log().toStdString() << "\n";
    return false;
  }

  return true;
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

bool
ShaderProgram::
fileToCode(const QString &filename, QString &code) const
{
  CFile file(filename.toStdString());

  if (! file.exists() || ! file.isRegular()) {
    std::cerr << "Invalid shader file '" << filename.toStdString() << "'\n";
    return false;
  }

  std::vector<std::string> lines;
  file.toLines(lines);

  for (const auto &line : lines) {
    int i = 0;

    auto skipSpace = [&]() {
      while (line[i] != '\0' && isspace(line[i]))
        ++i;
    };

    skipSpace();

    bool has_include = false;

    if (line[i] == '#') {
      ++i;

      skipSpace();

      auto len = line.size() - i;

      if (len >= 7 && strncmp(&line[i], "include", 7) == 0) {
        auto invalidLine = [&]() {
          std::cerr << "Invalid '#include' line '" + line << "'\n";
          return false;
        };

        has_include = true;

        i += 7;

        if (line[i] == '\0' || ! isspace(line[i]))
          return invalidLine();

        skipSpace();

        if (line[i] != '"')
          return invalidLine();

        ++i;

        auto j = i;

        while (line[i] != '\0' && line[i] != '"')
          ++i;

        auto ifile = line.substr(j, i - j);

        if (line[i] != '"')
          return invalidLine();

        ++i;

        QString code1;
        if (! fileToCode(QString::fromStdString(ifile), code1))
          return false;

        code += code1;
      }
    }

    if (! has_include)
      code += QString::fromStdString(line) + "\n";
  }

  return true;
}

//---

ShaderUniform::
ShaderUniform(ShaderProgram *program, const QString &name) :
 program_(program), name_(name) {
}

ShaderUniform::
~ShaderUniform()
{
}

int
ShaderUniform::
getLocation()
{
  if (location_ == -1) {
    location_ = program_->uniformLocation(name_.toLatin1().constData());

    if (location_ < 0) {
      std::cerr << "Invalid location for '" + name_.toStdString() << "'\n";
      return 0;
    }
  }

  return location_;
}

}
