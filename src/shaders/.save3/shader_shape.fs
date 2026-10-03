#version 330 core

in vec4 Color;
in vec2 TexCoord;

out vec3 color;

uniform sampler2D textureId;

void main() {
  vec3 c = texture(textureId, TexCoord).rgb;
  //color = Color.rgb + c;
  color = c;
}
