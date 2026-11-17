#version 330 core

#include "shaders/buffer_data.fs"

out vec4 FragColor;

//--- Lights
#include "shaders/single_light.fs"

uniform vec3 viewPos;

uniform bool lighted;

// --- Material
#include "shaders/material_global.fs"

//--- Skybox
uniform samplerCube cubeMap;
uniform bool        useCubeMap;

//--- Textures

#include "shaders/texture_data.fs"

//--- State

uniform bool pseudo;

uniform bool isInside;
uniform bool isPoint;
uniform bool isLine;

#include "shaders/wireframe_globals.fs"

uniform bool showOrient;
uniform vec3 orientColor;

uniform float opacity;

uniform bool  reflectionMap;
uniform float reflectivity;

uniform bool  refractionMap;
uniform float refractivity;

//---

#include "shaders/light_calc.fs"

//---

void main() {
  if (! pseudo) {
    // fixed color for state
    if      (isWireframe) {
      FragColor = vec4(wireframeColor, wireframeAlpha);
      return;
    }
    else if (isPoint) {
      FragColor = vec4(wireframeColor, wireframeAlpha);
      return;
    }
    else if (isLine) {
      FragColor = vec4(wireframeColor, wireframeAlpha);
      return;
    }
    else if (isInside) {
      FragColor = vec4(1.0, 1.0, 0.0, 0.1);
      return;
    }
  }

  //---

  // skip lighting
  if (! lighted) {
    FragColor = vec4(calcDiffuseColor(), 1);
    return;
  }

  //---

  if (showOrient) {
    FragColor = vec4(orientColor, 0.5);
    return;
  }

  //---

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

  // diffuse color
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

  FragColor = vec4(result, opacity);
}
