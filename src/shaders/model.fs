#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec3 Color;
in vec2 TexCoords;

out vec4 FragColor;

struct DirectionalLight {
  bool enabled;
  vec3 direction;
  vec3 color;
};

struct PointLight {
  bool  enabled;
  vec3  position;
  vec3  color;
  float radius;
};

struct SpotLight {
  bool  enabled;
  vec3  position;
  vec3  direction;
  vec3  color;
  float cutoff;
};

#define NUM_DIR_LIGHTS   1
#define NUM_POINT_LIGHTS 2
#define NUM_SPOT_LIGHTS  2

uniform DirectionalLight directionalLights[NUM_DIR_LIGHTS];
uniform PointLight       pointLights[NUM_POINT_LIGHTS];
uniform SpotLight        spotLights[NUM_SPOT_LIGHTS];

uniform vec3 viewPos;

struct TextureData {
  bool      enabled;
  sampler2D texture;
};

uniform TextureData diffuseTexture;
uniform TextureData specularTexture;
uniform TextureData normalTexture;

uniform float ambientStrength;
uniform float diffuseStrength;
uniform float specularStrength;
uniform float shininess;

uniform bool isWireframe;

float calcDiffuse(vec3 lightDir, vec3 nrm) {
  float diffAmt = max(0.0, dot(nrm, lightDir));
  return diffAmt;
}

float calcSpecular(vec3 lightDir, vec3 viewDir, vec3 nrm, float shininess) {
  vec3 halfVec = normalize(viewDir + lightDir);
  float specAmt = max(0.0, dot(halfVec, nrm));
  return pow(specAmt, shininess);
}

void main() {
  // normal
  vec3 norm;
  if (normalTexture.enabled) {
    norm = texture(normalTexture.texture, TexCoords).rgb;
    norm = normalize(norm*2.0 - 1.0); // this normal is in tangent space
  }
  else
    norm = normalize(Normal);

  // diffuse color
  vec3 diffuseColor;
  if (diffuseTexture.enabled)
    diffuseColor = texture(diffuseTexture.texture, TexCoords).rgb;
  else
    diffuseColor = Color;

  vec3 diffuse = diffuseStrength*diffuseColor;

  // specular color
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-viewDir, norm);
  vec3 specColor;
  if (specularTexture.enabled)
    specColor = texture(specularTexture.texture, TexCoords).rgb;
  else
    specColor = vec3(0, 0, 0);

  vec3 specular = specularStrength*specColor;

  // ambient
  vec3 ambient = ambientStrength*diffuseColor;

  // directional light
  vec3 result = vec3(0, 0, 0);

  for (int i = 0; i < NUM_DIR_LIGHTS; ++i) {
    DirectionalLight directionalLight = directionalLights[i];

    if (directionalLight.enabled) {
      //vec3 lightDir = normalize(directionalLight.position - FragPos);

      float diffAmt = calcDiffuse(directionalLight.direction, norm);
      float specAmt = calcSpecular(directionalLight.direction, viewDir, norm, shininess);

      result += diffAmt*directionalLight.color*diffuse + specAmt*directionalLight.color*specular;
    }
  }

  for (int i = 0; i < NUM_POINT_LIGHTS; ++i) {
    PointLight pointLight = pointLights[i];

    if (pointLight.enabled) {
      vec3 toLight = pointLight.position - FragPos;
      vec3 lightDir = normalize(toLight);
      float distToLight = length(toLight);
      float falloff = max(0.0, 1.0 - (distToLight/pointLight.radius));

      float diffAmt = calcDiffuse(lightDir, norm)*falloff;
      float specAmt = calcSpecular(lightDir, viewDir, norm, shininess)*falloff;

      result += diffAmt*pointLight.color*diffuse + specAmt*pointLight.color*specular;
    }
  }

  for (int i = 0; i < NUM_SPOT_LIGHTS; ++i) {
    SpotLight spotLight = spotLights[i];

    if (spotLight.enabled) {
      vec3 toLight = spotLight.position - FragPos;
      vec3 lightDir = normalize(toLight);
      float angle = dot(spotLight.direction, -lightDir);
      float falloff = (angle > spotLight.cutoff ? 1.0 : 0.0);

      float diffAmt = calcDiffuse(lightDir, norm)*falloff;
      float specAmt = calcSpecular(lightDir, viewDir, norm, shininess)*falloff;

      result += diffAmt*spotLight.color*diffuse + specAmt*spotLight.color*specular;
    }
  }

  result += ambient;

  FragColor = (isWireframe ? vec4(1.0, 1.0, 1.0, 1.0) : vec4(result, 1.0));
}
