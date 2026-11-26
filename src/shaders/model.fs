#version 330 core

#include "shaders/buffer_data.fs"

out vec4 FragColor;

//--- Lights

#include "shaders/multi_light.fs"

uniform vec3 viewPos;

// --- Material

#include "shaders/material_global.fs"

//--- Textures

#include "shaders/texture_data.fs"

//--- Shadows

#include "shaders/shadow_data.fs"

//--- State

#include "shaders/wireframe_globals.fs"

uniform float transparency;

//--- Outline

uniform bool isOutline;
uniform vec3 outlineColor;

//--- Reflection/Refraction

uniform samplerCube cubeMap;    // Skybox
uniform bool        useCubeMap;

uniform bool  reflectionMap;
uniform float reflectivity;

uniform bool  refractionMap;
uniform float refractivity;

//----

// Shadow Procs
#include "shaders/shadow_procs.fs"

// Light Procs
#include "shaders/light_calc.fs"
#include "shaders/light_type_calc.fs"

//---

void main() {
  if (isShadow) {
    return;
  }

  //---

  if      (isOutline) {
    FragColor = vec4(outlineColor, 1);
    return;
  }
  else if (isWireframe) {
    FragColor = vec4(wireframeColor, wireframeAlpha);
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

  vec3 result = ambient;

  bool lit = false;

  // lights
  for (int i = 0; i < NUM_LIGHTS; ++i) {
    if (! lights[i].enabled) {
      continue;
    }

    lit = true;

    if      (lights[i].type == 0) { // directional
      result += calcShadowDirectionalLight(lights[i], norm, diffuseColor, specColor, viewDir);
    }
    else if (lights[i].type == 1) { // point
      result += calcShadowPointLight(lights[i], norm, diffuseColor, specColor, viewDir);
    }
    else if (lights[i].type == 2) { // spot
      result += calcSpotLight(lights[i], norm, diffuseColor, specColor, viewDir);
    }
    else if (lights[i].type == 3) { // flashlight
      result += calcFlashLight(lights[i], norm, diffuseColor, specColor, viewDir);
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

  FragColor = vec4(result, transparency);

/*
  if (shadowMapDebug) {
    FragColor = ShadowColor;
  }
*/
}
