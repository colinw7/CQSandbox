#version 330 core

in vec4 FragPos;
in vec3 Normal;
in vec3 Color;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

out vec4 FragColor;

//--- Lights

// type:
//  0 : Directional (direction)
//  1 : Point (position)
//  2 : Spot (position, direction, cutoff, outer cutoff, exponent)
//  3 : FlashLight (cutoff, outer cutoff, exponent)
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
};

#define NUM_LIGHTS 5

uniform Light lights[NUM_LIGHTS];

uniform vec3 viewPos;

uniform vec3  ambientColor;
uniform float ambientStrength;
uniform float diffuseStrength;
uniform vec3  specularColor;
uniform float specularStrength;
uniform vec3  emissionColor;
uniform float emissiveStrength;
uniform float shininess;

//--- Shadows
uniform sampler2D shadowMap;
uniform bool      useShadowMap;
uniform bool      isShadow;

//--- Outline
uniform bool isOutline;
uniform vec3 outlineColor;

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

uniform bool isWireframe;

//---

vec3 calcNormal() {
  if (normalTexture.enabled) {
    vec3 norm = texture(normalTexture.texture, TexCoords).rgb;
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
  vec3 diffuseColor;
  if (diffuseTexture.enabled)
    diffuseColor = texture(diffuseTexture.texture, TexCoords).rgb;
  else
    diffuseColor = Color;

  vec3 diffuse = diffuseStrength*diffuseColor;

  return diffuse;
}

float calcSpecularFactor(vec3 lightDir, vec3 viewDir, vec3 nrm, float shininess) {
  vec3 halfVec = normalize(viewDir + lightDir);
  float specAmt = max(0.0, dot(halfVec, nrm));
  return pow(specAmt, shininess);
}

vec3 calcSpecularColor() {
  vec3 specColor;
  if (specularTexture.enabled)
    specColor = texture(specularTexture.texture, TexCoords).rgb;
  else
    specColor = vec3(0, 0, 0);

  vec3 specular = specularStrength*specColor;

  return specular;
}

vec3 calcEmissionColor() {
  vec3 emissionColor = vec3(0, 0, 0);
  if (emissiveTexture.enabled)
    return texture(emissiveTexture.texture, TexCoords).rgb;
  else
    return emissiveStrength*emissionColor;
}

//---

float shadowCalculation(vec4 fragPosLightSpace) {
  // perform perspective divide
  vec3 projCoords = fragPosLightSpace.xyz/fragPosLightSpace.w;

  // transform to [0,1] range
  projCoords = projCoords*0.5 + 0.5;

  // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
  float closestDepth = texture(shadowMap, projCoords.xy).r;

  // get depth of current fragment from light's perspective
  float currentDepth = projCoords.z;

  float bias = 0.00001;

  // check whether current frag pos is in shadow
  float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
//float shadow = currentDepth > closestDepth ? 1.0 : 0.0;

  return shadow;
}

//---

void main() {
  if (isShadow) {
    return;
  }

  if (isOutline) {
    FragColor = vec4(outlineColor, 1);
    return;
  }

  // normal
  vec3 norm = calcNormal();

  // ambient
  vec3 ambient = ambientStrength*ambientColor;

  vec3 result = ambient;

  // diffuse color
  vec3 diffuseColor = calcDiffuseColor();

  // specular color
  vec3 specColor = calcSpecularColor();

  vec3 viewDir = normalize(viewPos - vec3(FragPos));

  //vec3 reflectDir = reflect(-viewDir, norm);

  //---

  float shadow = 0.0;
  if (useShadowMap) {
    shadow = shadowCalculation(FragPosLightSpace);
  }

  //---

  bool lit = false;

  // lights
  for (int i = 0; i < NUM_LIGHTS; ++i) {
    if (! lights[i].enabled) {
      continue;
    }

    lit = true;

    if      (lights[i].type == 0) { // directional
      //vec3 lightDir = normalize(lights[i].position - vec3(FragPos));
      vec3 lightDir = normalize(-lights[i].direction);

      float diffAmt = calcDiffuseFactor(lightDir, norm);
      float specAmt = calcSpecularFactor(lightDir, viewDir, norm, shininess);

      result += (1 - shadow)*(diffAmt*lights[i].color*diffuseColor +
                              specAmt*lights[i].color*specColor);
    }
    else if (lights[i].type == 1) { // point
      vec3 toLight = lights[i].position - vec3(FragPos);
      vec3 lightDir = normalize(toLight);
      float distToLight = length(toLight);
      float falloff = max(0.0, 1.0 - (distToLight/lights[i].radius));

      float diffAmt = calcDiffuseFactor(lightDir, norm)*falloff;
      float specAmt = calcSpecularFactor(lightDir, viewDir, norm, shininess)*falloff;

      result += diffAmt*lights[i].color*diffuseColor + specAmt*lights[i].color*specColor;
    }
    else if (lights[i].type == 2) { // spot
      vec3 toLight = lights[i].position - vec3(FragPos);
      vec3 lightDir = normalize(toLight);
      float angle = dot(lights[i].direction, -lightDir);
      float falloff = (angle > lights[i].cutoff ? 1.0 : 0.0);

      float diffAmt = calcDiffuseFactor(lightDir, norm)*falloff;
      float specAmt = calcSpecularFactor(lightDir, viewDir, norm, shininess)*falloff;

      result += diffAmt*lights[i].color*diffuseColor + specAmt*lights[i].color*specColor;
    }
  }

  // baked diffuse lighting if none
  if (! lit) {
    float diffFactor = max(dot(norm, viewDir), 0.0);

    result += diffFactor*diffuseColor;
  }

  // add emission
  vec3 emissionColor = calcEmissionColor();

  result += emissionColor;

  // adjust color by state

  FragColor = (isWireframe ? vec4(1.0, 1.0, 1.0, 1.0) : vec4(result, 1.0));
}
