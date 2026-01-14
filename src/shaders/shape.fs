#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec4 Color;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float ambientStrength;
uniform float diffuseStrength;
uniform float specularStrength;
uniform float shininess;
uniform sampler2D textureId;
uniform sampler2D normTex;
uniform bool useDiffuseTexture;
uniform bool useNormalTexture;

void main() {
  vec3 norm;
  if (useNormalTexture) {
    norm = texture(normTex, TexCoord).rgb;
    norm = normalize(norm*2.0 - 1.0).rgb;
  } else {
    norm = normalize(Normal);
  }

  vec3 lightDir = normalize(lightPos - FragPos);

  float diff = max(dot(norm, lightDir), 0.0);
  vec4 diffuseColor = Color;
  if (useDiffuseTexture) {
    diffuseColor = texture(textureId, TexCoord);
  }
  vec3 diffuse = diffuseStrength*diff*vec3(diffuseColor);

  vec3 ambient = ambientStrength*vec3(diffuseColor);

  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, norm);

  float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
  vec3 specColor = lightColor;
  vec3 specular = specularStrength*spec*specColor;

  vec3 result = ambient + diffuse + specular;

  FragColor = vec4(result, diffuseColor.a);
}
