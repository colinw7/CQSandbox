#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexCoord;

uniform highp mat4 projection;
uniform highp mat4 view;
uniform highp mat4 model;

uniform bool isWireframe;
uniform bool isPoint;
uniform bool isLine;

uniform bool useFaceNormal;
uniform vec3 faceNormal;

uniform bool pseudo;

out vec4 FragPos;
out vec3 Normal;
out vec3 Color;
out vec2 TexCoord;

void main() {
  vec3 position = aPos;

  vec3 norm;
  if (useFaceNormal)
    norm = normalize(faceNormal);
  else
    norm = normalize(aNormal);

  FragPos = model*vec4(position, 1.0);
  Normal  = mat3(transpose(inverse(model)))*norm;

  Color    = aColor;
  TexCoord = aTexCoord;

  gl_Position = projection*view*FragPos;

  if (! pseudo) {
    if      (isWireframe)
      gl_Position += 0.01*vec4(norm, 1);
    else if (isPoint)
      gl_Position += 0.01*vec4(norm, 1);
    else if (isLine)
      gl_Position += 0.01*vec4(norm, 1);
  }
}
