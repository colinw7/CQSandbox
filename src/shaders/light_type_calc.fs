vec3 calcDirectionalLight(Light light, vec3 norm, vec3 diffuseColor,
                          vec3 specColor, vec3 viewDir) {
  float shadow = 0.0;

//vec3 lightDir = normalize(light.position - vec3(FragPos));
  vec3 lightDir = normalize(-light.direction);

  // diffuse light color
  float diffAmt = calcDiffuseFactor(lightDir, norm);

  // specular light color
  float specAmt = calcSpecularFactor(lightDir, viewDir, norm, shininess);

  return (1 - shadow)*(diffAmt*light.color*diffuseColor +
                       specAmt*light.color*specColor);
}

vec3 calcShadowDirectionalLight(Light light, vec3 norm, vec3 diffuseColor,
                                vec3 specColor, vec3 viewDir) {
  float shadow = 0.0;
  if (useShadowMap) {
    shadow = shadowCalculation(FragPosLightSpace[light.ind]);
  }

//vec3 lightDir = normalize(light.position - vec3(FragPos));
  vec3 lightDir = normalize(-light.direction);

  // diffuse light color
  float diffAmt = calcDiffuseFactor(lightDir, norm);

  // specular light color
  float specAmt = calcSpecularFactor(lightDir, viewDir, norm, shininess);

  return (1 - shadow)*(diffAmt*light.color*diffuseColor +
                       specAmt*light.color*specColor);
}

vec3 calcPointLight(Light light, vec3 norm, vec3 diffuseColor, vec3 specColor, vec3 viewDir) {
  vec3 toLight = light.position - vec3(FragPos);
  vec3 lightDir = normalize(toLight);

  // diffuse light color
  float falloff = 1.0;

  if (light.radius > 0.0) {
    float distToLight = length(toLight);

    /*
    falloff = 1.0/(light.attenuation0 +
                   distToLight*(light.attenuation1 +
                         distToLight*light.attenuation2));
    */
    falloff = max(0.0, 1.0 - (distToLight/light.radius));
  }
  
  float diffAmt = calcDiffuseFactor(lightDir, norm)*falloff;

  // specular light color
  float specAmt = calcSpecularFactor(lightDir, viewDir, norm, shininess)*falloff;

  return diffAmt*light.color*diffuseColor + specAmt*light.color*specColor;
}

vec3 calcShadowPointLight(Light light, vec3 norm, vec3 diffuseColor, vec3 specColor, vec3 viewDir) {
  float shadow = 0.0;
  if (useShadowCubeMap) {
    shadow = shadowCubeMapCalculation(vec3(FragPos), light.position);
    //return vec3(shadow, shadow, shadow);
    if (shadow < 0) return vec3(1, 0, 0);
    if (shadow > 1) return vec3(0, 1, 0);
  }

  vec3 toLight = light.position - vec3(FragPos);
  vec3 lightDir = normalize(toLight);

  // diffuse light color
  float falloff = 1.0;

  if (light.radius > 0.0) {
    float distToLight = length(toLight);

    /*
    falloff = 1.0/(light.attenuation0 +
                   distToLight*(light.attenuation1 +
                         distToLight*light.attenuation2));
    */
    falloff = max(0.0, 1.0 - (distToLight/light.radius));
  }

  float diffAmt = calcDiffuseFactor(lightDir, norm)*falloff;

  // specular light color
  float specAmt = calcSpecularFactor(lightDir, viewDir, norm, shininess);

  return (1 - shadow)*(diffAmt*light.color*diffuseColor +
                       specAmt*light.color*specColor);
}

vec3 calcSpotLight(Light light, vec3 norm, vec3 diffuseColor, vec3 specColor, vec3 viewDir) {
  vec3 toLight  = light.position - vec3(FragPos);
  vec3 lightDir = normalize(toLight);

  // diffuse light color
  float angle = dot(light.direction, -lightDir);

  // cos(0) = 1 (parallel), cos(90) = 0 (perp)
  // cutoff is cos(angle) so inside if greater

  float falloff = 0.0;

  if (light.outerCutoff < light.cutoff) {
    float epsilon = light.cutoff - light.outerCutoff;
    falloff = clamp((angle - light.outerCutoff)/epsilon, 0.0, 1.0);
  } else {
    falloff = clamp(angle/light.cutoff, 0.0, 1.0);
  }

  falloff = pow(falloff, light.exponent);
  //if (angle > light.cutoff) {
  //  falloff = pow(angle, light.exponent);
  //}

  float diffAmt = calcDiffuseFactor(lightDir, norm)*falloff;

  // specular light color
  float specAmt = calcSpecularFactor(lightDir, viewDir, norm, shininess)*falloff;

  return diffAmt*light.color*diffuseColor + specAmt*light.color*specColor;
}

vec3 calcFlashLight(Light light, vec3 norm, vec3 diffuseColor, vec3 specColor, vec3 viewDir) {
  vec3 toLight  = viewPos - vec3(FragPos);
  vec3 lightDir = normalize(toLight);

  // diffuse light color
  float angle = max(dot(lightDir, norm), 0.0);

  // cos(0) = 1 (parallel), cos(90) = 0 (perp)
  // cutoff is cos(angle) so inside if greater

  float falloff = 0.0;
  if (light.outerCutoff < light.cutoff) {
    float epsilon = light.cutoff - light.outerCutoff;
    falloff = clamp((angle - light.outerCutoff)/epsilon, 0.0, 1.0);
  } else {
    falloff = clamp(angle/light.cutoff, 0.0, 1.0);
  }
  falloff = pow(falloff, light.exponent);

  //if (angle > light.cutoff) {
  //  falloff = pow(angle, light.exponent);
  //}

  float diffAmt = calcDiffuseFactor(lightDir, norm)*falloff;

  // specular light color
  float specAmt = calcSpecularFactor(lightDir, viewDir, norm, shininess);

  return diffAmt*light.color*diffuseColor + specAmt*light.color*specColor;
}
