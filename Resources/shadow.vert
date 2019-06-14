#version 330

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 6) in vec2 texCoord;

out vec3 passNormal;
out vec2 passTexCoord;

void main()
{
    
    //vec3 tempPosition = vec3(0.f, 20, 20.f);
    mat4 tempViewMatrix = mat4(1.f, 0.f, 0.f, 0.f, 0.f, 0.707107f, 0.707107f, 0.f, 0.f, -0.707107f, 0.707107f, 0.f, 0.f, 0.f, -28.2843f, 1.f);
    //mat4 tempProjectionMatrix = mat4(0.8391, 0, 0, 0, 0, 0.8391, 0, 0, 0, 0, -1.002, -0.2002, 0, 0, -1, 0);
    
//    [0.8391, 0, 0, 0]
//    [0, 0.8391, 0, 0]
//    [0, 0, -1.002, -0.2002]
//    [0, 0, -1, 0]
    
//    [1, 0, 0, 0]
//    [0, 0.707107, -0.707107, 0]
//    [0, 0.707107, 0.707107, -28.2843]
//    [0, 0, 0, 1]
    
    gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(position, 1.f);
    //gl_Position = projMatrix * modelMatrix * vec4(position, 1.f);
    
    passNormal = (modelMatrix * vec4(normal, 0)).xyz;
    passTexCoord = texCoord;
}
