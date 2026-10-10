#version 330 core

out vec4 FragColor;

uniform vec3  selectColor;
uniform float selectAlpha;
uniform bool  isWireframe;

void main() {
  if (isWireframe)
    FragColor = vec4(selectColor, selectAlpha);
  else
    FragColor = vec4(selectColor, 0.1);
}
