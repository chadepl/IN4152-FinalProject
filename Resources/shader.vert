#version 330

uniform bool forTesting;

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 diffuseColor;
layout(location = 3) in vec3 ambientColor;
layout(location = 4) in vec3 specularColor;
layout(location = 5) in float shininessValue;
layout(location = 6) in vec2 texCoord;

out vec3 passPosition;
out vec3 passNormal;
out vec2 passTexCoord;
out vec4 passShadowCoord;
//out vec3 passColor;

out struct Material {
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
} material;

void main()
{
    gl_Position = projMatrix * viewMatrix * modelMatrix * position;
    
    passPosition = (modelMatrix * position).xyz;
    passNormal = (modelMatrix * vec4(normal, 0)).xyz;
    passTexCoord = texCoord;
    
    if(!forTesting) { // delete when finished
        material.diffuseColor = diffuseColor;
        material.ambientColor = ambientColor;
        material.specularColor = specularColor;
        material.shininess = shininessValue;
    } else {
        material.diffuseColor = vec3(1.0f, 0.5f, 0.31f);
        material.ambientColor = vec3(1.0f, 0.5f, 0.31f);
        material.specularColor = vec3(1.0f, 0.5f, 0.31f);
        material.shininess = 256.f;
    }
    
}
