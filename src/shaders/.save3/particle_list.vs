#version 330 core

attribute highp vec4 position;
attribute highp vec4 center;
attribute lowp vec4 color;

uniform highp mat4 projection;
uniform highp mat4 view;
uniform highp mat4 model;

varying lowp vec4 col;
varying highp vec2 texPos;

void main() {
  col = color;
  texPos = position.xy + 0.5;
  gl_Position = (projection*view*model*center) + 0.05*position;
}
