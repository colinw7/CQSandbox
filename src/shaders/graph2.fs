#version 330 core

uniform vec3 lineColor;

void main()
{
  gl_FragColor = vec4(lineColor.x, lineColor.y, lineColor.z, 1.0f);
}
