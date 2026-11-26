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

//--- State

#include "shaders/wireframe_globals.fs"

//-- Lights

#include "shaders/light_calc.fs"
#include "shaders/light_type_calc.fs"

//---

void main() {
  if (isWireframe) {
    FragColor = vec4(wireframeColor, wireframeAlpha);
    return;
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
      result += calcDirectionalLight(lights[i], norm, diffuseColor, specColor, viewDir);
    }
    else if (lights[i].type == 1) { // point
      result += calcPointLight(lights[i], norm, diffuseColor, specColor, viewDir);
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

  FragColor = vec4(result, 1.0f);
}
