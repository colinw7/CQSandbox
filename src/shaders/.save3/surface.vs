#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

uniform highp mat4 projection;
uniform highp mat4 view;
uniform highp mat4 model;

out vec4 FragPos;
out vec3 Normal;
out vec3 Color;

void main() {
  FragPos = model*vec4(aPos, 1.0);
  Normal  = mat3(transpose(inverse(model)))*aNormal;
  Color   = aColor;

  gl_Position = projection*view*FragPos;
}
