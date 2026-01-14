#ifndef ShaderToyProgram_H
#define ShaderToyProgram_H

#include <QObject>
#include <QString>

namespace CQSandbox {

class ShaderProgram;

class ShaderToyProgram {
 public:
  ShaderToyProgram(QObject *parent=nullptr);

  ShaderProgram *program() const { return program_; }

  void setFragmentShader(const QString &str);
  void setVertexShader(const QString &str);

  bool isTexture() const { return texture_; }
  void setTexture(bool b) { texture_ = b; }

  void updateShader();

  void setShaderToyUniforms(int w, int h, float elapsed, int ticks);

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
