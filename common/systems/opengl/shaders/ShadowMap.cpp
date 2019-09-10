namespace kengine::Shaders::src {
    namespace ShadowMap {
        const char * frag = R"(
#version 330

uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;
uniform float shadow_map_min_bias;
uniform float shadow_map_max_bias;

vec2 getShadowMapValue(vec3 worldPos) {
    vec4 worldPosLightSpace = lightSpaceMatrix * vec4(worldPos, 1.0);
    vec3 projCoords = worldPosLightSpace.xyz / worldPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // transform to [0,1] range

    float objectDepth = projCoords.z;
    float shadowMapValue = texture(shadowMap, projCoords.xy).r;
    return vec2(shadowMapValue, objectDepth);
}

float calcShadow(vec3 worldPos, vec3 normal, vec3 lightDir) {
    vec4 worldPosLightSpace = lightSpaceMatrix * vec4(worldPos, 1.0);
    vec3 projCoords = worldPosLightSpace.xyz / worldPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // transform to [0,1] range

    float currentDepth = projCoords.z;
    float closestDepth = texture(shadowMap, projCoords.xy).r;

    // calculate bias (based on depth map resolution and slope)
    float bias = max(shadow_map_max_bias * (1.0 - dot(normal, lightDir)), shadow_map_min_bias);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    if (projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}
        )";
    }
}