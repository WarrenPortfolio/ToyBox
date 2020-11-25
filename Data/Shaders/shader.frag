#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    vec3 cameraPosition;
    float ambientLightIntensity;
    vec3  ambientLightColor;
} ubo;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

void main()
{
    // diffuse
    vec4 diffuseColor = texture(texSampler, fragTexCoord);

    // normal
    vec3 normal = normalize(fragNormal);

    // ambient lighting
    vec3    ambientColor    = ubo.ambientLightIntensity * ubo.ambientLightColor;

    // light
    vec3    lightColor      = vec3(1.0, 1.0, 1.0);
    vec3    lightDir        = normalize(ubo.cameraPosition - fragPos);
    float   lightDifference = max(dot(normal, lightDir), 0.0);
    vec3    lightValue      = lightDifference * lightColor;

    // specular highlight
    float specularStrength  = 0.5;
    float shininess         = 32;
    vec3    viewPos         = ubo.cameraPosition;
    vec3    viewDir         = normalize(viewPos - fragPos);
    vec3    reflectDir      = reflect(-lightDir, normal);
    float   spec            = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3    specularColor   = specularStrength * spec * lightColor;

    outColor = diffuseColor;
    outColor.xyz = (ambientColor + lightValue + specularColor) * diffuseColor.xyz;
}