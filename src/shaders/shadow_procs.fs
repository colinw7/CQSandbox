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
