#ifndef CQSandboxShaderProgram_H
#define CQSandboxShaderProgram_H

#include <CQSandboxUtil.h>
#include <CQGLUtil.h>
#include <CPoint2D.h>

#include <QObject>
#include <QOpenGLShaderProgram>

#include <GL/gl.h>

class CQGLBuffer;

namespace CQSandbox {

class ShaderProgram;

class ShaderUniform {
 public:
  ShaderUniform(ShaderProgram *program, const QString &name);

  virtual ~ShaderUniform();

  virtual void loadValue() = 0;

 protected:
  int getLocation();

 protected:
  ShaderProgram* program_ { nullptr };
  QString        name_;
  int            location_ { -1 };
};

//---

class ShaderProgram {
 public:
  ShaderProgram(QObject *parent=nullptr);

  bool addVertexFile  (const QString &filename);
  bool addGeometryFile(const QString &filename);
  bool addFragmentFile(const QString &filename);

  bool addVertexCode  (const QString &code, const QString &filename="");
  bool addGeometryCode(const QString &code, const QString &filename="");
  bool addFragmentCode(const QString &code, const QString &filename="");

  QOpenGLShaderProgram *program() const { return program_; }

  CQGLBuffer *createBuffer() const;

  void link();
  void bind();
  void release();

  //---

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

  void setUniformValue(int name, const double &value) {
    program_->setUniformValue(name, float(value));
  }

  void setUniformValue(int name, const CRGBA &value) {
    program_->setUniformValue(name, CQGLUtil::toVector(value));
  }

  void setUniformValue(const char *name, const std::vector<double> &value) {
    for (uint i = 0; i < value.size(); ++i) {
      auto aname = QString("%1[%2]").arg(name).arg(i);

      program_->setUniformValue(aname.toLatin1().constData(), float(value[i]));
    }
  }

  void setUniformValue(const char *name, const std::vector<CPoint2D> &value) {
    for (uint i = 0; i < value.size(); ++i) {
      auto aname = QString("%1[%2]").arg(name).arg(i);

      program_->setUniformValue(aname.toLatin1().constData(), Util::toQPointF(value[i]));
    }
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
  void setUniformValue(int name, const T &value1, const T &value2,
                       const T &value3, const T &value4) {
    program_->setUniformValue(name, value1, value2, value3, value4);
  }

  template<typename T>
  void setUniformValueArray(const char *name, const T *values, int count) {
    program_->setUniformValueArray(name, values, count);
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

  //---

  void addUniform(ShaderUniform *uniform) {
    uniforms_.push_back(uniform);
  }

  void loadUniforms() {
    for (auto *uniform : uniforms_)
      uniform->loadValue();
  }

 private:
  bool fileToCode(const QString &filename, QString &code) const;

 private:
  QOpenGLShaderProgram* program_ { nullptr };

  GLint projectionUniform_ { 0 };
  GLint viewUniform_       { 0 };

  QString vertexFile_;
  QString geometryFile_;
  QString fragmentFile_;

  std::vector<ShaderUniform *> uniforms_;
};

//---

template<typename T>
class ShaderUniformT : public ShaderUniform {
 public:
  ShaderUniformT(ShaderProgram *program, const QString &name) :
   ShaderUniform(program, name) {
  }

  void setValue(const T &value) { value_ = value; }

  void loadValue() override {
    program_->setUniformValue(getLocation(), value_);
  }

 private:
  T value_ {};
};

template<typename T>
class ShaderUniformArrayT : public ShaderUniform {
 public:
  ShaderUniformArrayT(ShaderProgram *program, const QString &name) :
   ShaderUniform(program, name) {
  }

  void setValue(const std::vector<T> &value) { value_ = value; }

  void loadValue() override {
    program_->setUniformValue(name_.toLatin1().constData(), value_);
  }

 private:
  std::vector<T> value_;
};

}

#endif
