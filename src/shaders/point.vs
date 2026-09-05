#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform highp mat4 projection;
uniform highp mat4 view;

out vec3 Color;

void main() {
  Color = aColor;

  gl_Position = projection*view*vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
