#version 330 core

in vec4 Color;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D textureId;
uniform bool useTexture;

void main() {
  if (useTexture) {
    FragColor = texture(textureId, TexCoord);
  } else {
    FragColor = Color;
  }
}
