#version 330 core

in vec4 FragPos;
in vec3 Normal;
in vec3 Color;
in vec2 TexCoords;

out vec4 FragColor;

//--- Lights

struct DirectionalLight {
  bool enabled;
  vec3 color;
  vec3 direction;
};

struct PointLight {
  bool  enabled;
  vec3  color;
  vec3  position;
  float radius;
};

struct SpotLight {
  bool  enabled;
  vec3  color;
  vec3  position;
  vec3  direction;
  float cutoff;
};

#define NUM_DIR_LIGHTS   1
#define NUM_POINT_LIGHTS 2
#define NUM_SPOT_LIGHTS  2

uniform DirectionalLight directionalLights[NUM_DIR_LIGHTS];
uniform PointLight       pointLights[NUM_POINT_LIGHTS];
uniform SpotLight        spotLights[NUM_SPOT_LIGHTS];

uniform vec3 viewPos;

uniform vec3  ambientColor;
uniform float ambientStrength;
uniform float diffuseStrength;
uniform vec3  specularColor;
uniform float specularStrength;
uniform vec3  emissionColor;
uniform float emissiveStrength;
uniform float shininess;

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

void main() {
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

  bool lit = false;

  // directional lights
  for (int i = 0; i < NUM_DIR_LIGHTS; ++i) {
    DirectionalLight directionalLight = directionalLights[i];

    if (directionalLight.enabled) {
      //vec3 lightDir = normalize(directionalLight.position - vec3(FragPos));

      float diffAmt = calcDiffuseFactor(directionalLight.direction, norm);
      float specAmt = calcSpecularFactor(directionalLight.direction, viewDir, norm, shininess);

      result += diffAmt*directionalLight.color*diffuseColor +
                specAmt*directionalLight.color*specColor;

      lit = true;
    }
  }

  // point lights
  for (int i = 0; i < NUM_POINT_LIGHTS; ++i) {
    PointLight pointLight = pointLights[i];

    if (pointLight.enabled) {
      vec3 toLight = pointLight.position - vec3(FragPos);
      vec3 lightDir = normalize(toLight);
      float distToLight = length(toLight);
      float falloff = max(0.0, 1.0 - (distToLight/pointLight.radius));

      float diffAmt = calcDiffuseFactor(lightDir, norm)*falloff;
      float specAmt = calcSpecularFactor(lightDir, viewDir, norm, shininess)*falloff;

      result += diffAmt*pointLight.color*diffuseColor + specAmt*pointLight.color*specColor;

      lit = true;
    }
  }

  // spot lights
  for (int i = 0; i < NUM_SPOT_LIGHTS; ++i) {
    SpotLight spotLight = spotLights[i];

    if (spotLight.enabled) {
      vec3 toLight = spotLight.position - vec3(FragPos);
      vec3 lightDir = normalize(toLight);
      float angle = dot(spotLight.direction, -lightDir);
      float falloff = (angle > spotLight.cutoff ? 1.0 : 0.0);

      float diffAmt = calcDiffuseFactor(lightDir, norm)*falloff;
      float specAmt = calcSpecularFactor(lightDir, viewDir, norm, shininess)*falloff;

      result += diffAmt*spotLight.color*diffuseColor + specAmt*spotLight.color*specColor;

      lit = true;
    }
  }

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
