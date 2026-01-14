#version 330 core

layout (location = 0) in vec3 point;

uniform highp mat4 projection;
uniform highp mat4 view;
uniform highp mat4 model;

void main()
{
  gl_Position = projection * view * model * vec4(point.x, point.y, point.z, 1.0);
}
