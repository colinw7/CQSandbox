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
