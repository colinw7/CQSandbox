#version 330 core

#include "shaders/buffer_data.vs"

#include "shaders/world_matrix.vs"

uniform bool isWireframe;
uniform bool isPoint;
uniform bool isLine;

uniform bool useFaceNormal;
uniform vec3 faceNormal;

uniform bool pseudo;

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
