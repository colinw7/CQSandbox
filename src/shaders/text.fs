#version 330 core

varying lowp vec3 col;
varying highp vec2 uv0;

uniform sampler2D textureId;

void main() {
  vec4 tc = texture(textureId, uv0);

  gl_FragColor = vec4(col.r, col.g, col.b, tc.r);
}
