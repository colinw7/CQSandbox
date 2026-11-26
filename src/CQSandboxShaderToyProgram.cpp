#include <CQSandboxShaderToyProgram.h>
#include <CQSandboxShaderProgram.h>

namespace CQSandbox {

ShaderToyProgram::
ShaderToyProgram(App *app) :
 app_(app)
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
uniform vec3      iCameraPos;\n\
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

  program_ = new ShaderProgram(app_);

  program_->addVertexCode  (vertexShader);
  program_->addFragmentCode(fragmentShader);

  program_->link();

  //---

  shaderValid_ = true;
}

void
ShaderToyProgram::
setShaderToyUniforms(const UniformData &data)
{
  auto *program = this->program();

  int elapsedLocation = program->uniformLocation("iTime");
  program->setUniformValue(elapsedLocation, GLfloat(data.elapsed));

  //---

  int frameLocation = program->uniformLocation("iFrame");
  program->setUniformValue(frameLocation, GLint(data.ticks));

  //---

  int resLocation = program->uniformLocation("iResolution");
  program->setUniformValue(resLocation, GLfloat(data.w), GLfloat(data.h), 1.0f);

  //---

  //      mouse.xy  = mouse position during last button down
  //  abs(mouse.zw) = mouse position during last button click
  // sign(mouze.z)  = button is down
  // sign(mouze.w)  = button is clicked
  int mouseLocation = program->uniformLocation("iMouse");
  auto mx = data.mouseMove.x;
  auto my = data.h - 1 - data.mouseMove.y;
  auto px = data.mousePress.x;
  auto py = data.h - 1 - data.mousePress.y;

  program->setUniformValue(mouseLocation,
    GLfloat(mx), GLfloat(my), GLfloat(px), GLfloat(py));

  //---

  int cameraPosLocation = program->uniformLocation("iCameraPos");
  program->setUniformValue(cameraPosLocation,
    GLfloat(data.cameraPos.x), GLfloat(data.cameraPos.y), GLfloat(data.cameraPos.z));
}

}
