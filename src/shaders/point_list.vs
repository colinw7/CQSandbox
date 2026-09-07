#version 330 core

attribute highp vec4 position; // shape point (centered at 0,0)
attribute highp vec4 center;   // point center
attribute lowp  vec4 color;

uniform highp mat4 projection;
uniform highp mat4 view;
uniform highp mat4 model;

uniform highp vec3 cameraUp;
uniform highp vec3 cameraRight;

uniform highp float pointSize;

varying lowp  vec4 col;
varying highp vec2 texPos;

void main() {
  col = color;

  texPos = position.xy + 0.5;

  vec3 position1 = (cameraRight*position.x*pointSize) + (cameraUp*position.y*pointSize);

  gl_Position = (projection*view*model*center) + vec4(position1, 1);
}
