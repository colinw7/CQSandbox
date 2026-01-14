#version 330 core

varying lowp vec4 col;
varying highp vec2 texPos;

uniform bool useTexture;
uniform sampler2D textureId;

void main() {
  if (useTexture) {
    vec4 tc = texture(textureId, texPos);
    if (tc.a < 0.1) {
      discard;
    }
    gl_FragColor = col*tc;
  } else {
    gl_FragColor = col;
  }
}
