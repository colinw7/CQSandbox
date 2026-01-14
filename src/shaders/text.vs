#version 330 core

attribute highp vec4 position;
attribute highp vec2 texCoord0;
attribute lowp  vec3 color;

uniform highp mat4 projection;
uniform highp mat4 view;
uniform highp mat4 model;

varying highp vec2 uv0;
varying lowp vec3 col;

void main() {
  gl_Position = projection * view * model * position;

  uv0 = texCoord0;
  col = color;
}
