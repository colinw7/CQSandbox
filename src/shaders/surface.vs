#version 330 core

#include "shaders/buffer_data.fs"

#include "shaders/world_matrix.vs"

void main() {
  vec3 position = aPos;
  vec3 norm     = normalize(aNormal);

  FragPos = model*vec4(position, 1.0);
  Normal  = mat3(transpose(inverse(model)))*norm;

  Color    = aColor;
  TexCoord = aTexCoord;

  gl_Position = projection*view*FragPos;
}
