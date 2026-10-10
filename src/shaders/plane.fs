#version 330 core

in vec4 Color;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D textureId;
uniform bool      useTexture;

uniform bool  isWireframe;
uniform vec3  wireframeColor;
uniform float wireframeAlpha;
uniform bool  isPoint;
uniform bool  isLine;

void main() {
    if      (isWireframe)
    FragColor = vec4(wireframeColor, wireframeAlpha);
  else if (isPoint)
    FragColor = vec4(wireframeColor, wireframeAlpha);
  else if (isLine)
    FragColor = vec4(wireframeColor, wireframeAlpha);
  else {
    if (useTexture) {
      FragColor = texture(textureId, TexCoord);
    } else {
      FragColor = Color;
    }
  }
}
