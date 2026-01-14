#version 330 core

layout (location = 0) in vec3 aPos;

uniform highp mat4 projection;
uniform highp mat4 view;
uniform highp mat4 model;

void main() {
  gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
