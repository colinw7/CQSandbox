#version 330 core

in vec4 FragPos;
in vec3 Normal;
in vec3 Color;
in vec2 TexCoord;

out vec4 FragColor;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;

// --- Material
uniform vec3  ambientColor;
uniform float ambientStrength;
uniform float diffuseStrength;
uniform vec3  specularColor;
uniform float specularStrength;
uniform float shininess;

//--- Textures

struct TextureData {
  bool      enabled;
  sampler2D texture;
};

uniform TextureData diffuseTexture;

//--- State

#include "shaders/wireframe_globals.fs"

//---

float calcDiffuseFactor(vec3 lightDir, vec3 nrm) {
  float diffAmt = max(0.0, dot(nrm, lightDir));
  return diffAmt;
}

vec3 calcDiffuseColor() {
  vec3 diffColor;
  if (diffuseTexture.enabled)
    diffColor = texture(diffuseTexture.texture, TexCoord).rgb;
  else
    diffColor = Color;

  vec3 diffuse = diffuseStrength*diffColor;

  return diffuse;
}

float calcSpecularFactor(vec3 lightDir, vec3 viewDir, vec3 nrm, float shininess) {
  vec3 reflectDir = reflect(-lightDir, nrm);
  float specAmt = max(0.0, dot(viewDir, reflectDir));
  
  return pow(specAmt, shininess);
}

vec3 calcSpecularColor() {
  return specularStrength*specularColor;
}

//---

void main() {
  if (isWireframe) {
    FragColor = vec4(wireframeColor, wireframeAlpha);
    return;
  }

  //---

  // normal
  vec3 norm = normalize(Normal);

  // view direction
  vec3 viewDir = normalize(viewPos - vec3(FragPos));

  //---

  // global colors

  // ambient
  vec3 ambient = ambientStrength*ambientColor;

  // diffuse color
  vec3 diffuseColor = calcDiffuseColor();

  // specular color
  vec3 specColor = calcSpecularColor();

  //---

  vec3 result = ambient;

  // diffuse and specular color (per light)
  vec3 lightDir = normalize(lightPos - vec3(FragPos));

  float diffAmt = calcDiffuseFactor(lightDir, norm);
  float specAmt = calcSpecularFactor(lightDir, viewDir, norm, shininess);

  result += diffAmt*lightColor*diffuseColor +
            specAmt*lightColor*specColor;

  //---

//FragColor = vec4(result, 1.0f);
  FragColor = vec4(diffuseColor, 1.0f);
}
