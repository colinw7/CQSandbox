#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec4 aTexCoord;

uniform highp mat4 projection;
uniform highp mat4 view;
uniform highp mat4 model;

out vec4 Color;
out vec2 TexCoord;

void main() {
  Color    = aColor;
  TexCoord = vec2(aTexCoord.x, aTexCoord.y);

  gl_Position = projection * view * model * vec4(aPos, 1.0);
}
