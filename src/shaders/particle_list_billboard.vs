#version 330 core

attribute highp vec3 position;
attribute highp vec3 normal;
attribute highp vec3 center;
attribute lowp  vec4 color;

uniform highp mat4 projection;
uniform highp mat4 view;
uniform highp mat4 model;

uniform highp vec3 cameraUp;
uniform highp vec3 cameraRight;

uniform highp float particleSize;
uniform lowp  bool  particleFlat;

varying lowp  vec4 Color;
varying highp vec3 Normal;
varying highp vec4 FragPos;
varying highp vec2 TexPos;

void main() {
  Color  = color;
  Normal = normal;

  FragPos = model*vec4(center, 1);
  TexPos  = position.xy + 0.5;

  if (particleFlat) {
    vec3 position1 = (cameraRight*position.x*particleSize) + (cameraUp*position.y*particleSize);

    gl_Position = (projection*view*FragPos) + vec4(position1, 1);
  } else {
    gl_Position = (projection*view*FragPos) + vec4(position, 1)*particleSize;
  }
}
