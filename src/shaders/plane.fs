#version 330 core

in vec3 Color;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D textureId;
uniform bool      useTexture;

#include "shaders/wireframe_globals.fs"

uniform float alpha;
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
      FragColor = vec4(Color, alpha);
    }
  }
}
