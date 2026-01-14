#include <CQSandboxShaderToyProgram.h>
#include <CQSandboxShaderProgram.h>

namespace CQSandbox {

ShaderToyProgram::
ShaderToyProgram(QObject *parent) :
 parent_(parent)
{
  fragmentShader_ = QString("\
void mainImage(out vec4 fragColor, in vec2 fragCoord) {\n\
  vec2 uv = fragCoord/iResolution;\n\
  fragColor = vec4(uv.x, uv.y, 0.0, 1.0);\n\
}\n\
");

  vertexShader_ = QString("\
attribute vec4 a_Position;\n\
attribute vec2 a_Coordinates;\n\
\n\
void main() {\n\
  gl_Position = vec4(a_Coordinates.x, a_Coordinates.y, 1.0f, 1.0f);\n\
}");
}

void
ShaderToyProgram::
setVertexShader(const QString &str)
{
  vertexShader_ = str;
  shaderValid_  = false;
}

void
ShaderToyProgram::
setFragmentShader(const QString &str)
{
  fragmentShader_ = str;
  shaderValid_    = false;
}

void
ShaderToyProgram::
updateShader()
{
  if (shaderValid_)
    return;

  //---

  // fragment shader is standard uniforms, specified shader string, and mainImage call
  auto fragmentShader = QString("\
#version 330\n\
\n\
uniform vec3      iResolution;\n\
uniform float     iTime;\n\
uniform float     iTimeDelta;\n\
uniform float     iFrameRate;\n\
uniform int       iFrame;\n\
uniform float     iChannelTime[4];\n\
uniform vec4      iMouse;\n\
uniform vec4      iDate;\n\
uniform vec4      iSampleRate;\n\
uniform vec3      iChannelResolution[4];\n\
uniform sampler2D iChannel0;\n\
uniform sampler2D iChannel1;\n\
uniform sampler2D iChannel2;\n\
uniform sampler2D iChannel3;\n\
\n");

  if (isTexture()) {
    fragmentShader += "\
layout (location = 0) out vec3 color;\n\
\n";
  }

  fragmentShader += fragmentShader_;

  if (isTexture()) {
    fragmentShader += QString("\n\
void main() {\n\
  vec4 c;\n\
  mainImage(c, gl_FragCoord.xy);\n\
  color = c.rgb;\n\
}\n");
  }
  else {
    fragmentShader += QString("\n\
void main() {\n\
  mainImage(gl_FragColor, gl_FragCoord.xy);\n\
}\n");
  }

  //---

  // vertex shader is specified shader string
  auto vertexShader = QString("\
#version 330\n\
\n");

  vertexShader += vertexShader_;

  //---

  program_ = new ShaderProgram(parent_);

  program_->addVertexCode  (vertexShader);
  program_->addFragmentCode(fragmentShader);

  program_->link();

  //---

  shaderValid_ = true;
}

void
ShaderToyProgram::
setShaderToyUniforms(int w, int h, float elapsed, int ticks)
{
  auto *program = this->program();

  int elapsedLocation = program->uniformLocation("iTime");
  program->setUniformValue(elapsedLocation, GLfloat(elapsed));

  //---

  int frameLocation = program->uniformLocation("iFrame");
  program->setUniformValue(frameLocation, GLint(ticks));

  //---

  int resLocation = program->uniformLocation("iResolution");

  program->setUniformValue(resLocation, GLfloat(w), GLfloat(h), 1.0f);
}

}
