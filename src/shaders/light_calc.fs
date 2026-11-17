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
  vec3 diffColor;
  if (diffuseTexture.enabled)
    diffColor = texture(diffuseTexture.texture, TexCoord).rgb;
  else
    diffColor = Color;

  vec3 diffuse = diffuseStrength*diffColor;

  return diffuse;
}

float calcSpecularFactor(vec3 lightDir, vec3 viewDir, vec3 nrm, float shininess) {
  //vec3 reflectDir = reflect(-lightDir, nrm);
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
