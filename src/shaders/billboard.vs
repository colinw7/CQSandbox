#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 billboardPosition;
uniform vec2 billboardSize;

out vec2 TexCoords;

void main()
{
  vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
  vec3 cameraUp = vec3(view[0][1], view[1][1], view[2][1]);

  vec3 vertexPosition = billboardPosition + (cameraRight * aPos.x * billboardSize.x) + (cameraUp * aPos.y * billboardSize.y);

  gl_Position = projection * view * vec4(vertexPosition, 1.0);
  TexCoords = aTexCoords;
}
