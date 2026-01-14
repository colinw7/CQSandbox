#version 330 core

layout (location = 1) in vec3 line;

uniform highp mat4 projection;
uniform highp mat4 view;
uniform highp mat4 model;

void main()
{
  gl_Position = projection * view * model * vec4(line.x, line.y, line.z, 1.0);
}
