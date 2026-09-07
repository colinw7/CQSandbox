#version 330 core

varying lowp  vec4 Color;
varying highp vec4 FragPos;
varying highp vec2 TexPos;

uniform bool      useTexture;
uniform sampler2D textureId;

uniform bool  particleFlat;
uniform float particleAlpha;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;

uniform vec3  ambientColor;
uniform float ambientStrength;
uniform float diffuseStrength;
uniform float specularStrength;
uniform float shininess;

void main() {
  if (useTexture) {
    vec4  tc = texture(textureId, TexPos);
    float ta = 0.1;

    if (tc.a < ta) {
      discard;
    }

    gl_FragColor = Color*tc;
  } else {
    if (particleFlat) {
      float r = 0.5;

      if (length(TexPos - vec2(r, r)) > r) {
        discard;
      }

      gl_FragColor = vec4(Color.rgb, particleAlpha);
    } else {
      vec3 lightDir = normalize(lightPos - vec3(FragPos));
      vec3 viewDir  = normalize(viewPos  - vec3(FragPos));

      float angle = dot(lightDir, viewDir);

      gl_FragColor = angle*Color;
    }
  }
}
