#version 330

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out vec3 passNormal;
out vec2 passTexCoord;

void main()
{
    //gl_Position = projMatrix * viewMatrix * modelMatrix * position;
    mat4 myModelMatrix = mat4(100., 0., 0., 0., 0., 100., 0., 0., 0., 0., 100., 0., 0., 0., 0., 1.);
    gl_Position = projMatrix * viewMatrix * myModelMatrix * position;
    
    passNormal = (modelMatrix * vec4(normal, 0)).xyz;
    passTexCoord = texCoord;
    
}
