#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec3 Color;
in vec2 TexCoords1;
in vec2 TexCoords2;

out vec4 FragColor;

uniform vec3 viewPos;

uniform vec3 lightPos;
uniform vec3 lightColor;

uniform float ambientStrength;
uniform float diffuseStrength;
uniform float specularStrength;
uniform float shininess;

struct TextureData {
  bool      enabled;
  sampler2D texture;
};

uniform TextureData diffuseTexture;
uniform TextureData specularTexture;
uniform TextureData normalTexture;

uniform bool isWireframe;

void main()
{
  // normal
  vec3 norm;
  if (normalTexture.enabled) {
    vec3 norm1 = texture(normalTexture.texture, TexCoords1).rgb;
    norm1 = normalize(norm1*2.0 - 1.0); // this normal is in tangent space
    vec3 norm2 = texture(normalTexture.texture, TexCoords2).rgb;
    norm2 = normalize(norm2*2.0 - 1.0); // this normal is in tangent space
    norm = normalize(norm1 + norm2);
  }
  else
    norm = normalize(Normal);

  // diffuse
  vec3 lightDir = normalize(lightPos - FragPos);
  float diff = max(dot(norm, lightDir), 0.0);

  vec3 diffuseColor;
  if (diffuseTexture.enabled)
    diffuseColor = texture(diffuseTexture.texture, TexCoords1).rgb;
  else
    diffuseColor = Color;
  vec3 diffuse = diffuseStrength*diff*diffuseColor;

  // ambient
  vec3 ambient = ambientStrength*diffuseColor;

  // specular
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, norm);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
  vec3 specColor;
  if (specularTexture.enabled)
    specColor = texture(specularTexture.texture, TexCoords1).rgb;
  else
    specColor = lightColor;
  vec3 specular = specularStrength*spec*specColor;

  vec3 result = ambient + diffuse + specular;

  FragColor = (isWireframe ? vec4(1.0, 1.0, 1.0, 1.0) : vec4(result, 1.0));
}
