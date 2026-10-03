#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;

uniform highp mat4 projection;
uniform highp mat4 view;
uniform highp mat4 model;

out vec4 Color;

void main() {
  Color = aColor;

  gl_Position = projection*view*model*vec4(aPos, 1.0);
}
