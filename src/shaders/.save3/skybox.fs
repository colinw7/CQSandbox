#version 330 core

uniform samplerCube textureId;
uniform bool isWireframe;

in vec3 TexCoord;

out vec4 FragColor;

void main() {
  if (isWireframe) {
    FragColor = vec4(TexCoord, 1.0);
  } else {
    FragColor = texture(textureId, TexCoord);
  }
}
