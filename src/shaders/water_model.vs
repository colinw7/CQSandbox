#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexCoord;

uniform highp mat4 projection;
uniform highp mat4 view;
uniform highp mat4 model;

out vec3 FragPos;
out vec3 Normal;
out vec3 Color;
out vec2 TexCoords1;
out vec2 TexCoords2;

uniform float ticks;

void main()
{
  FragPos    = vec3(model * vec4(aPos, 1.0));
  Normal     = mat3(transpose(inverse(model)))*aNormal;
  Color      = aColor;
  TexCoords1 = vec2(aTexCoord.x + 0.2*ticks, aTexCoord.y + 0.3*ticks);
  TexCoords2 = vec2(aTexCoord.x - 0.3*ticks, aTexCoord.y - 0.2*ticks);

  gl_Position = projection * view * vec4(FragPos, 1.0);
}