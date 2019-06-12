#version 330

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 color;

out vec3 passPosition;
out vec3 passNormal;
out vec2 passTexCoord;
out vec4 passShadowCoord;
out vec3 passColor;

void main()
{
    gl_Position = projMatrix * viewMatrix * modelMatrix * position;
    
    passPosition = (modelMatrix * position).xyz;
    passNormal = (modelMatrix * vec4(normal, 0)).xyz;
    passTexCoord = texCoord;
    passColor = color;
}