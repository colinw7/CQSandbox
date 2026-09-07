#version 330 core

attribute highp vec4 position;
attribute highp vec4 center;
attribute lowp  vec4 color;

uniform highp mat4 projection;
uniform highp mat4 view;
uniform highp mat4 model;

uniform highp vec3 cameraUp;
uniform highp vec3 cameraRight;

uniform highp float particleSize;
uniform lowp  bool  particleFlat;

varying lowp  vec4 Color;
varying highp vec4 FragPos;
varying highp vec2 TexPos;

void main() {
  Color = color;

  FragPos = model*center;
  TexPos  = position.xy + 0.5;

  if (particleFlat) {
    vec3 position1 = (cameraRight*position.x*particleSize) + (cameraUp*position.y*particleSize);

    gl_Position = (projection*view*FragPos) + vec4(position1, 1);
  } else {
    gl_Position = (projection*view*FragPos) + position*particleSize;
  }
}
