#ifndef ShaderToyProgram_H
#define ShaderToyProgram_H

#include <CPoint2D.h>

#include <QObject>
#include <QString>

namespace CQSandbox {

class ShaderProgram;

class ShaderToyProgram {
 public:
  struct UniformData {
    int      w       { 100 };
    int      h       { 100 };
    float    elapsed { 0.0f };
    int      ticks   { 0 };
    CPoint2D mousePress;
    CPoint2D mouseMove ;
  };

 public:
  ShaderToyProgram(QObject *parent=nullptr);

  ShaderProgram *program() const { return program_; }

  void setFragmentShader(const QString &str);
  void setVertexShader(const QString &str);

  bool isTexture() const { return texture_; }
  void setTexture(bool b) { texture_ = b; }

  void updateShader();

  void setShaderToyUniforms(const UniformData &data);

 private:
  QObject *parent_ { nullptr };

  ShaderProgram* program_ { nullptr };

  QString fragmentShader_;
  QString vertexShader_;
  bool    texture_ { false };
  bool    shaderValid_ { false };
};

}

#endif
