#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(std140, binding = 0) uniform UniformBufferObject
{
    mat4 view;
    mat4 proj;
    vec3 cameraPosition;
    
    float ambientLightIntensity;
    vec3  ambientLightColor;
    
    float directionalLightIntensity;
    vec3  directionalLightColor;
    vec3  directionalLightDirection;

    vec3  materialColor;
    vec3  materialSpecularColor;
    float materialRoughness;
} ubo;

layout(std140, push_constant) uniform UniformPushConstant 
{
    mat4 model;
} upc;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragColor;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragNormal;

layout(location = 0) out vec4 outColor;

void main()
{
    // diffuse
    vec4    diffuseColor    = texture(texSampler, fragTexCoord) * vec4(ubo.materialColor, 1.0f);
    
    // normal
    vec3    normal          = normalize(fragNormal);

    // ambient lighting
    vec3    ambientColor    = ubo.ambientLightColor * ubo.ambientLightIntensity;

    // light
    vec3    lightColor      = ubo.directionalLightColor * ubo.directionalLightIntensity;
    vec3    lightDir        = normalize(-ubo.directionalLightDirection);
    float   lightDifference = max(dot(normal, lightDir), 0.0);
    vec3    lightValue      = lightDifference * lightColor;

    // specular highlight (blinn-phong)
    float shininess         = min(2048, max(0.001, (2.0 / pow(ubo.materialRoughness, 2))));
    vec3    viewPos         = ubo.cameraPosition;
    vec3    viewDir         = normalize(viewPos - fragPos);
    vec3    halfDir         = normalize(lightDir + viewDir);
    float   specAngle       = max(0.0, dot(halfDir, normal));
    float   specular        = pow(specAngle,  shininess);
    vec3    specularColor   = lightColor * ubo.materialSpecularColor * specular;

    outColor.xyz            = (ambientColor + lightValue + specularColor) * diffuseColor.xyz;
    outColor.a              = diffuseColor.a;
}