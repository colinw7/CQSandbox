#ifndef CQSandboxShaderProgram_H
#define CQSandboxShaderProgram_H

#include <QObject>
#include <QOpenGLShaderProgram>

#include <GL/gl.h>

class CQGLBuffer;

namespace CQSandbox {

class ShaderProgram {
 public:
  ShaderProgram(QObject *parent=nullptr);

  void addVertexFile  (const QString &filename);
  void addGeometryFile(const QString &filename);
  void addFragmentFile(const QString &filename);

  void addVertexCode  (const QString &code);
  void addGeometryCode(const QString &code);
  void addFragmentCode(const QString &code);

  QOpenGLShaderProgram *program() const { return program_; }

  CQGLBuffer *createBuffer() const;

  void link();
  void bind();
  void release();

  void setProjectionUniform(const char *name="projection") {
    projectionUniform_ = uniformLocation(name);
    Q_ASSERT(projectionUniform_ != -1);
  }

  void setViewUniform(const char *name="view") {
    viewUniform_ = uniformLocation(name);
    Q_ASSERT(viewUniform_ != -1);
  }

  template<typename T>
  void setUniformValue(const char *name, const T &value) {
    program_->setUniformValue(name, value);
  }

  template<typename T>
  void setUniformValue(int name, const T &value) {
    program_->setUniformValue(name, value);
  }

  template<typename T>
  void setUniformValue(int name, const T &value1, const T &value2) {
    program_->setUniformValue(name, value1, value2);
  }

  template<typename T>
  void setUniformValue(int name, const T &value1, const T &value2, const T &value3) {
    program_->setUniformValue(name, value1, value2, value3);
  }

  template<typename T>
  void setAttributeArray(int name, const T &value) {
    program_->setAttributeArray(name, value);
  }

  void enableAttributeArray(int i) {
    program_->enableAttributeArray(i);
  }

  void disableAttributeArray(int i) {
    program_->disableAttributeArray(i);
  }

  int attributeLocation(const char *name) {
    return program_->attributeLocation(name);
  }

  int uniformLocation(const char *name) {
    return program_->uniformLocation(name);
  }

 private:
  QOpenGLShaderProgram* program_ { nullptr };

  GLint projectionUniform_ { 0 };
  GLint viewUniform_       { 0 };
};

}

#endif
