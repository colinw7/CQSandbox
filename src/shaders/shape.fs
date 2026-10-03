#version 330 core

in vec4 FragPos;
in vec3 Normal;
in vec3 Color;
in vec2 TexCoord;

out vec4 FragColor;

//--- Lights

struct Light {
  int   type;
  bool  enabled;
  vec3  position;
  vec3  direction;
  vec3  color;
  float radius;
  float cutoff;
  float outerCutoff;
  float exponent;
  float attenuation0;
  float attenuation1;
  float attenuation2;
  float power;
};

uniform Light light;

uniform vec3 viewPos;

// --- Material
uniform vec3  ambientColor;
uniform float ambientStrength;
uniform float diffuseStrength;
uniform vec3  specularColor;
uniform float specularStrength;
uniform vec3  emissionColor;
uniform float emissiveStrength;
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
uniform TextureData specularTexture;
uniform TextureData emissiveTexture;

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
  if (specularTexture.enabled)
    return texture(specularTexture.texture, TexCoord).rgb;

  return specularStrength*specularColor;
}

vec3 calcEmissionColor() {
  if (emissiveTexture.enabled)
    return texture(emissiveTexture.texture, TexCoord).rgb;

  return emissiveStrength*emissionColor;
}

//---

void main() {
  vec3 reflectColor = vec3(0, 0, 0);
  vec3 refractColor = vec3(0, 0, 0);

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

  if (reflectionMap && reflectivity > 0) {
    //specColor = vec3(reflectColor);
    diffuseColor = (1 - reflectivity)*diffuseColor + reflectivity*reflectColor;
  }

  if (refractionMap && refractivity > 0) {
    //specColor = vec3(refractColor);
    diffuseColor = (1 - refractivity)*diffuseColor + refractivity*refractColor;
  }

  //---

  float shadow = 0.0;

  //---

  vec3 result = ambient;

  // diffuse and specular color (per light)
/*
  vec3 lightDir = normalize(light.position - vec3(FragPos));

  float diffAmt = calcDiffuseFactor(-lightDir, norm);
  float specAmt = calcSpecularFactor(lightDir, viewDir, norm, shininess);

  result += diffAmt*light.color*diffuseColor*light.power +
            specAmt*light.color*specColor;
*/
  if      (light.type == 0) { // directional
    //vec3 lightDir = normalize(light.position - vec3(FragPos));
    vec3 lightDir = normalize(-light.direction);

    float diffAmt = calcDiffuseFactor(lightDir, norm);
    float specAmt = calcSpecularFactor(lightDir, viewDir, norm, shininess);

    result += (1 - shadow)*(diffAmt*light.color*diffuseColor +
                            specAmt*light.color*specColor);
  }
  else if (light.type == 1) { // point
    vec3 toLight = light.position - vec3(FragPos);
    vec3 lightDir = normalize(toLight);
    float distToLight = length(toLight);
    float falloff = max(0.0, 1.0 - (distToLight/light.radius));
  
    float diffAmt = calcDiffuseFactor(lightDir, norm)*falloff;
    float specAmt = calcSpecularFactor(lightDir, viewDir, norm, shininess)*falloff;

    result += diffAmt*light.color*diffuseColor + specAmt*light.color*specColor;
  }
  else if (light.type == 2) { // spot
    vec3 toLight = light.position - vec3(FragPos);
    vec3 lightDir = normalize(toLight);
    float angle = dot(light.direction, -lightDir);
    float falloff = (angle > light.cutoff ? 1.0 : 0.0);

    float diffAmt = calcDiffuseFactor(lightDir, norm)*falloff;
    float specAmt = calcSpecularFactor(lightDir, viewDir, norm, shininess)*falloff;

    result += diffAmt*light.color*diffuseColor + specAmt*light.color*specColor;
  }

  //---

  // add emission
  vec3 emissionColor = calcEmissionColor();

  result += emissionColor;

  //---

  // adjust color by state

  FragColor = (isWireframe ? vec4(1.0, 1.0, 1.0, 1.0) : vec4(result, opacity));
}
