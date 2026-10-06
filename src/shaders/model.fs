#version 330 core

in vec4 FragPos;
in vec3 Normal;
in vec3 Color;
in vec2 TexCoord;

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
  float power;
  mat4  matrix;
};

#define NUM_LIGHTS 5

uniform Light lights[NUM_LIGHTS];

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

//--- Shadows

in  vec4 FragPosLightSpace[NUM_LIGHTS]; // for directional shadow
out vec4 ShadowColor;                   // for cube map shadow debug

uniform float shadowBias;
uniform float far_plane;

// Directional Shadow
uniform sampler2D shadowMap;
uniform bool      useShadowMap;
uniform bool      isShadow;

// Point Cube Map Shadow
uniform samplerCube shadowCubeMap;
uniform bool        useShadowCubeMap;
//uniform bool        shadowMapDebug;

//--- Outline

uniform bool isOutline;
uniform vec3 outlineColor;

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
uniform float transparency;

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
  vec3 diffuseColor;
  if (diffuseTexture.enabled)
    diffuseColor = texture(diffuseTexture.texture, TexCoord).rgb;
  else
    diffuseColor = Color;

  vec3 diffuse = diffuseStrength*diffuseColor;

  return diffuse;
}

float calcSpecularFactor(vec3 lightDir, vec3 viewDir, vec3 nrm, float shininess) {
  //vec3 reflectDir = reflect(-viewDir, norm);
  //float specAmt = max(0.0, dot(viewDir, reflectDir));

  vec3 halfVec = normalize(viewDir + lightDir);
  float specAmt = max(0.0, dot(halfVec, nrm));

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

float shadowCubeMapCalculation(vec3 fragPos, vec3 lightPos) {
  // get vector between fragment position and light position
  vec3 fragToLight = fragPos - lightPos;

  // use the fragment to light vector to sample from the depth map
  float closestDepth = texture(shadowCubeMap, fragToLight).r;
  //float closestDepth = 0.0;

  // it is currently in linear range between [0,1],
  // let's re-transform it back to original depth value
  closestDepth *= far_plane;

  // now get current linear depth as the length between the fragment and light position
  float currentDepth = length(fragToLight);

  // test for shadows
  // we use a much larger bias since depth is now in [near_plane, far_plane] range
//float bias = 0.05;
  float bias = shadowBias;

  float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

  // display closestDepth as debug (to visualize depth cubemap)
  ShadowColor = vec4(vec3(closestDepth/far_plane), 1.0);

  return shadow;
}

float shadowCalculation(vec4 fragPosLightSpace) {
  // perform perspective divide
  vec3 projCoord = fragPosLightSpace.xyz/fragPosLightSpace.w;

  // transform to [0,1] range
  projCoord = projCoord*0.5 + 0.5;

  // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
  float closestDepth = texture(shadowMap, projCoord.xy).r;

  // get depth of current fragment from light's perspective
  float currentDepth = projCoord.z;

// calculate bias (based on depth map resolution and slope)
/*
  vec3 normal = normalize(Normal);
  vec3 lightDir = normalize(lightPos - FragPos);
  float bias = max(0.05*(1.0 - dot(normal, lightDir)), 0.005);
*/
  float bias = shadowBias;

  // check whether current frag pos is in shadow
  float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
//float shadow = currentDepth > closestDepth ? 1.0 : 0.0;

/*
  // PCF
  float shadow = 0.0;
  vec2 texelSize = 1.0/textureSize(shadowMap, 0);
  for (int x = -1; x <= 1; ++x) {
    for (int y = -1; y <= 1; ++y) {
      float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y)*texelSize).r;
      shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
    }
  }
  shadow /= 9.0;
*/

  //float shadow = currentDepth - shadowBias > closestDepth ? 1.0 : 0.0;
  //float shadow = currentDepth > closestDepth ? 1.0 : 0.0;

  return shadow;
}

//---

vec3 calcDirectionalLight(int i, vec3 norm, vec3 diffuseColor, vec3 specColor, vec3 viewDir) {
  float shadow = 0.0;
  if (useShadowMap) {
    shadow = shadowCalculation(FragPosLightSpace[i]);
  }

  //vec3 lightDir = normalize(lights[i].position - vec3(FragPos));
  vec3 lightDir = normalize(-lights[i].direction);

  // diffuse light color
  float diffAmt = calcDiffuseFactor(lightDir, norm);

  // specular light color
  float specAmt = calcSpecularFactor(lightDir, viewDir, norm, shininess);

  return (1 - shadow)*(diffAmt*lights[i].color*diffuseColor +
                       specAmt*lights[i].color*specColor);
}

vec3 calcPointLight(int i, vec3 norm, vec3 diffuseColor, vec3 specColor, vec3 viewDir) {
  float shadow = 0.0;
  if (useShadowCubeMap) {
    shadow = shadowCubeMapCalculation(vec3(FragPos), lights[i].position);
    //return vec3(shadow, shadow, shadow);
    if (shadow < 0) return vec3(1, 0, 0);
    if (shadow > 1) return vec3(0, 1, 0);
  }

  vec3 toLight = lights[i].position - vec3(FragPos);
  vec3 lightDir = normalize(toLight);

  // diffuse light color
  float falloff = 1.0;

  if (lights[i].radius > 0.0) {
    float distToLight = length(toLight);

    /*
    falloff = 1.0/(lights[i].attenuation0 +
                   distToLight*(lights[i].attenuation1 +
                         distToLight*lights[i].attenuation2));
    */
    falloff = max(0.0, 1.0 - (distToLight/lights[i].radius));
  }

  float diffAmt = calcDiffuseFactor(lightDir, norm)*falloff;

  // specular light color
  float specAmt = calcSpecularFactor(lightDir, viewDir, norm, shininess);

  return (1 - shadow)*(diffAmt*lights[i].color*diffuseColor +
                       specAmt*lights[i].color*specColor);
}

vec3 calcSpotLight(int i, vec3 norm, vec3 diffuseColor, vec3 specColor, vec3 viewDir) {
  vec3 toLight  = lights[i].position - vec3(FragPos);
  vec3 lightDir = normalize(toLight);

  float diffFactor = 1.0; 

  vec3 lightDir1 = normalize(-lights[i].direction);

  // diffuse light color
  float angle = dot(lights[i].direction, -lightDir);

  // cos(0) = 1 (parallel), cos(90) = 0 (perp)
  // cutoff is cos(angle) so inside if greater
  
  float falloff = 0.0;
  if (lights[i].outerCutoff < lights[i].cutoff) {
    float epsilon = lights[i].cutoff - lights[i].outerCutoff;
    falloff = clamp((angle - lights[i].outerCutoff)/epsilon, 0.0, 1.0);
  } else {
    falloff = clamp(angle/lights[i].cutoff, 0.0, 1.0);
  }
  falloff = pow(falloff, lights[i].exponent);
  
  //if (angle > lights[i].cutoff) {
  //  falloff = pow(angle, lights[i].exponent);
  //}

  float diffAmt = calcDiffuseFactor(lightDir, norm)*falloff;

  // specular light color
  float specAmt = calcSpecularFactor(lightDir, viewDir, norm, shininess);

  return diffAmt*lights[i].color*diffuseColor + specAmt*lights[i].color*specColor;
}

vec3 calcFlashLight(int i, vec3 norm, vec3 diffuseColor, vec3 specColor, vec3 viewDir) {
  vec3 toLight  = viewPos - vec3(FragPos);
  vec3 lightDir = normalize(toLight);

  // diffuse light color
  float angle = max(dot(lightDir, norm), 0.0);

  // cos(0) = 1 (parallel), cos(90) = 0 (perp)
  // cutoff is cos(angle) so inside if greater

  float falloff = 0.0;
  if (lights[i].outerCutoff < lights[i].cutoff) {
    float epsilon = lights[i].cutoff - lights[i].outerCutoff;
    falloff = clamp((angle - lights[i].outerCutoff)/epsilon, 0.0, 1.0);
  } else {
    falloff = clamp(angle/lights[i].cutoff, 0.0, 1.0);
  }
  falloff = pow(falloff, lights[i].exponent);

  //if (angle > lights[i].cutoff) {
  //  falloff = pow(angle, lights[i].exponent);
  //}

  float diffAmt = calcDiffuseFactor(lightDir, norm)*falloff;

  // specular light color
  float specAmt = calcSpecularFactor(lightDir, viewDir, norm, shininess);

  return diffAmt*lights[i].color*diffuseColor + specAmt*lights[i].color*specColor;
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

  //---

  vec3 reflectColor = vec3(0, 0, 0);
  vec3 refractColor = vec3(0, 0, 0);

  if (reflectionMap) {
    vec3 I = normalize(vec3(FragPos) - viewPos);
    vec3 R = reflect(I, normalize(Normal));

    reflectColor = texture(cubeMap, R).rgb;
  }

  if (refractionMap) {
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

  // diffuse color
  vec3 diffuseColor = calcDiffuseColor();

  // specular color
  vec3 specColor = calcSpecularColor();

  if     (reflectionMap) {
    //specColor = reflectColor;
    diffuseColor = (1 - reflectivity)*diffuseColor + reflectivity*reflectColor;
  }
  else if (refractionMap) {
    //specColor = refractColor;
    diffuseColor = (1 - refractivity)*diffuseColor + refractivity*refractColor;
  }

  //---

  vec3 result = ambient;

  bool lit = false;

  // lights
  for (int i = 0; i < NUM_LIGHTS; ++i) {
    if (! lights[i].enabled) {
      continue;
    }

    lit = true;

    if      (lights[i].type == 0) { // directional
      result += calcDirectionalLight(i, norm, diffuseColor, specColor, viewDir);
    }
    else if (lights[i].type == 1) { // point
      result += calcPointLight(i, norm, diffuseColor, specColor, viewDir);
    }
    else if (lights[i].type == 2) { // spot
      result += calcSpotLight(i, norm, diffuseColor, specColor, viewDir);
    }
    else if (lights[i].type == 3) { // flashlight
      result += calcFlashLight(i, norm, diffuseColor, specColor, viewDir);
    }
  }

  // baked diffuse lighting if none
  if (! lit) {
    float diffFactor = max(dot(norm, viewDir), 0.0);

    result += diffFactor*diffuseColor;
  }

  //---

  // add emission
  vec3 emissionColor = calcEmissionColor();

  result += emissionColor;

  //---

  // adjust color by state

  FragColor = (isWireframe ? vec4(1.0, 1.0, 1.0, 1.0) : vec4(result, transparency));

/*
  if (shadowMapDebug) {
    FragColor = ShadowColor;
  }
*/
}
