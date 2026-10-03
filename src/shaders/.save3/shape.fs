#version 330 core

in vec4 FragPos;
in vec3 Normal;
in vec3 Color;
in vec2 TexCoord;

out vec4 FragColor;

//--- Lights

uniform vec3  lightPos;
uniform vec3  lightColor;
uniform float lightPower;

uniform vec3 viewPos;

uniform vec3  ambientColor;
uniform float ambientStrength;
uniform float diffuseStrength;
uniform vec3  specularColor;
uniform float specularStrength;
uniform float shininess;

//--- Skybox
uniform samplerCube cubeMap;
uniform bool        useCubeMap;

//--- Textures

struct TextureData {
  bool      enabled;
  sampler2D texture;
};

uniform TextureData diffuseTexture;
uniform TextureData normalTexture;

//--- State

uniform bool  isWireframe;
uniform float opacity;

uniform bool  reflectionMap;
uniform float reflectivity;

uniform bool  refractionMap;
uniform float refractivity;

//---

vec3 calcNormal() {
  if (normalTexture.enabled) {
    vec3 norm = texture(normalTexture.texture, TexCoord).rgb;
    norm = normalize(norm*2.0 - 1.0); // this normal is in tangent space
    return norm;
  }
  else
    return normalize(Normal);
}

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

void main() {
  vec3 reflectColor = vec3(1, 0, 0);
  vec3 refractColor = vec3(0, 1, 0);

  if (reflectionMap && reflectivity > 0) {
    vec3 I = normalize(vec3(FragPos) - viewPos);
    vec3 R = reflect(I, normalize(Normal));

    reflectColor = texture(cubeMap, R).rgb;
  }

  if (refractionMap && refractivity > 0) {
    float ratio = 1.00/1.52; // glass

    vec3 I = normalize(vec3(FragPos) - viewPos);
    vec3 R = refract(I, normalize(Normal), ratio);

    refractColor = texture(cubeMap, R).rgb;
  }

  //---

  // normal
  vec3 norm = calcNormal();

  // view direction
  vec3 viewDir = normalize(viewPos - vec3(FragPos));

  //---

  // global colors

  // ambient
  vec3 ambient = ambientStrength*ambientColor;

  // diffuse colot
  vec3 diffuseColor = calcDiffuseColor();

  // specular color
  vec3 specColor = calcSpecularColor();

  vec3 diffuseColor1 = diffuseColor;

  if (reflectionMap && reflectivity > 0) {
    //specColor = vec3(reflectColor);
    diffuseColor1 = (1 - reflectivity)*diffuseColor + reflectivity*reflectColor;
  }

  if (refractionMap && refractivity > 0) {
    //specColor = vec3(refractColor);
    diffuseColor1 = (1 - refractivity)*diffuseColor + refractivity*refractColor;
  }

  //---

  vec3 result = ambient;

  // diffuse color (per light)
  vec3 lightDir = normalize(lightPos - vec3(FragPos));

  float diffAmt = calcDiffuseFactor(-lightDir, norm);

  result += diffAmt*lightColor*diffuseColor1*lightPower;

  //---

  // specular color (per light)
  float specAmt = calcSpecularFactor(lightDir, viewDir, norm, shininess);

  result += specAmt*lightColor*specColor;

  //---

  if (! isWireframe)
    FragColor = vec4(result, opacity);
  else
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
